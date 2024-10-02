#include "./DummySinModel.h"
// #include "core/Rack.h"
// #include "ext/dr_wav.h" //not used but now we have it..
// #include <iostream>
#include <vector>

namespace Synth::DummySin {

// Constructor to accept buffer and size
Model::Model(float *audioBuffer, std::size_t bufferSize)
    : buffer(audioBuffer), bufferSize(bufferSize) {
    setupParams(); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    SynthInterface::initializeParameters();
    SynthInterface::setupCCmapping("DummySin"); // Adjust path as needed
    reset();
}

void Model::setupParams() {
    if (SynthInterface::parameterDefinitions.empty()) {
        SynthInterface::parameterDefinitions = {
            {"pan", {0.1f, 0, false, 0, 1, [this](float v) {
                         gainLeft = AudioMath::ccos(v * 0.25f);  // Left gain decreases as panVal goes to 1
                         gainRight = AudioMath::csin(v * 0.25f); // Right gain increases as panVal goes to 1
                     }}},
            {"cutoff", {0.5f, 0, true, 50, 8, [this](float v) {
                            std::cout << "Setting cutoff to " << v << std::endl;
                            cutoffHz = v;
                            initLPF();
                        }}},
            {"detune", {0.5f, 0, false, -100, 100, [this](float v) {
                            // note that attributes are from-to.
                            std::cout << "Setting detune to " << v << std::endl;
                            // cutoffHz = v;
                            // initLPF();
                        }}},
            {"filter_mode", {0.0f, 3, false, 0, 2, [this](float v) {
                                 // go rough and just use numer, or cast to emum.
                                 // maybe enum just half baked so skip..
                                 filterType = FilterType::LPF;
                                 initLPF();
                             }}}};
    }
}

void Model::reset() {
    // dunno..
    // make sawtooth..
    for (int i = 1; i < 14; i++) {
        this->applySine(i, 0.5 / i);
    }
    /*
    this->applySine(1, 0.5);
    this->applySine(2, 0.4);
    this->applySine(3, 0.3);
    this->applySine(4, 0.2);
    this->applySine(5, 0.1);
    this->applySine(6, 0.1);
    this->applySine(7, 0.1);
    */
}

void Model::parseMidi(char cmd, char param1, char param2) {
    u_int8_t messageType = static_cast<uint8_t>(cmd & 0xf0);
    float fParam2 = static_cast<float>(param2) * (1.0f / 127.0f);
    switch (messageType) {
    case 0x80:
        // Note off, only bother if match to note playing..
        if (param1 == notePlaying) {
            vcaLevelTarget = 0.0f;
        }
        break;
    case 0x90:
        // Note on
        notePlaying = param1;
        // bend cents needs to be calculated on render..
        vcaLevelTarget = 1.0f;
        break;
    case 0xb0:
        // Control change (CC)
        SynthInterface::handleMidiCC(static_cast<int>(param1), fParam2);
        break;
    case 0xe0: {
        // Pitch bend extra varialbes - use curly braces..
        int pitchBendValue = (static_cast<int>(param2) << 7) | static_cast<int>(param1);
        int pitchBendCentered = pitchBendValue - 8192;
        std::cout << "PB:" << pitchBendCentered << std::endl;
        // Scale the pitch bend to +/- 100 cents
        bendCents = (pitchBendCentered / 8192.0f) * 200.0f;
        break;
    }
    default:
        // Handle other messages
        break;
    }
}

void Model::applySine(float multiple, float amplitude) {
    for (size_t i = 0; i < LUT_SIZE; ++i) {
        float rad = (2.0f * M_PI * i * multiple / LUT_SIZE);
        lut1[i] += std::sin(rad) * amplitude;
    }
}

void Model::initLPF() {
    float RC = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float dt = 1.0f / TPH_DSP_SR;
    alpha = dt / (RC + dt);
}

void Model::initBPF() {
    // Example calculation for BPF
    float RC1 = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float RC2 = 1.0f / (2.0f * M_PI * (this->cutoffHz * 1.5f)); // Example bandwidth
    float dt = 1.0f / TPH_DSP_SR;
    alpha = dt / (RC1 + dt);
    // You may need to store more states for the BPF filter
}

void Model::initHPF() {
    float RC = 1.0f / (2.0f * M_PI * this->cutoffHz);
    float dt = 1.0f / TPH_DSP_SR;
    alpha = RC / (RC + dt);
}

bool Model::renderNextBlock() {
    // we could declare this signal as mono. would be good to try.
    float hz = 0;
    hz = AudioMath::noteToHz(notePlaying, bendCents);
    angle = hz * LUT_SIZE * (1.0f / TPH_DSP_SR);
    // note that unlike JUCE, we produce both channels at once. When ever more efficient..
    for (std::size_t i = 0; i < bufferSize; i += 2) {
        lutIdx = lutIdx + angle;
        if (lutIdx >= LUT_SIZE) {
            lutIdx -= LUT_SIZE;
        }
        // LUT out of bounds: int intIdx = static_cast<int>(lutIdx + (lutIdx < 0 ? -0.5f : 0.5f));
        int intIdx = static_cast<int>(lutIdx);
        float y = lut1[intIdx] * 0.5;
        buffer[i] = y * this->gainLeft * vcaLevel;
        buffer[i + 1] = y * this->gainRight * vcaLevel;
        vcaLevel += vcaLevelDelta;
    }
    switch (this->filterType) {
    case FilterType::LPF:
        for (std::size_t i = 0; i < bufferSize; i += 2) {
            float leftInput = buffer[i];
            float rightInput = buffer[i + 1];
            buffer[i] = alpha * leftInput + (1.0f - alpha) * previousLeft;
            previousLeft = buffer[i];
            buffer[i + 1] = alpha * rightInput + (1.0f - alpha) * previousRight;
            previousRight = buffer[i + 1];
        }
        break;
    case FilterType::BPF:
        // Simple Band Pass Filter Implementation (can be refined)
        for (std::size_t i = 0; i < bufferSize; i += 2) {
            float leftInput = buffer[i];
            float rightInput = buffer[i + 1];
            buffer[i] = alpha * (leftInput - previousLeft) + previousLeft;
            previousLeft = buffer[i];
            buffer[i + 1] = alpha * (rightInput - previousRight) + previousRight;
            previousRight = buffer[i + 1];
        }
        break;
    case FilterType::HPF:
        for (std::size_t i = 0; i < bufferSize; i += 2) {
            float leftInput = buffer[i];
            float rightInput = buffer[i + 1];
            buffer[i] = alpha * (previousLeft + leftInput - buffer[i]);
            previousLeft = buffer[i];
            buffer[i + 1] = alpha * (previousRight + rightInput - buffer[i + 1]);
            previousRight = buffer[i + 1];
        }
        break;
    }

    // update vcaLevelDelta
    vcaLevelDelta = (vcaLevelTarget - vcaLevel) * 0.015; // 0.15=64=>0.96!

    return true;
}

} // namespace Synth::DummySin
