#include "./DummyModel.h"
// #include "core/Rack.h"
// #include "ext/dr_wav.h" //not used but now we have it..
// #include <iostream>
#include <vector>

namespace Synth::Dummy {

// Constructor to accept buffer and size
Model::Model(float *audioBuffer, std::size_t bufferSize)
    : buffer(audioBuffer), bufferSize(bufferSize) {
    setupParams(); // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    SynthInterface::initializeParameters();
    SynthInterface::setupCCmapping("Dummy"); // Adjust path as needed
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
}

void Model::parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) {
    u_int8_t messageType = static_cast<uint8_t>(cmd & 0xf0);
    float fParam2 = static_cast<float>(param2) * (1.0f / 127.0f);

    switch (messageType) {
    case 0x80:
        // Note off
        break;
    case 0x90:
        // Note on
        noiseVolume = 0.6;
        break;
    case 0xb0:
        // Control change (CC)
        SynthInterface::handleMidiCC(static_cast<int>(param1), fParam2);
        break;
    default:
        // Handle other messages
        break;
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
    for (std::size_t i = 0; i < bufferSize; i += 2) {
        buffer[i] = AudioMath::noise() * noiseVolume * this->gainLeft;
        buffer[i + 1] = AudioMath::noise() * noiseVolume * this->gainRight;
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
    //
    if (noiseVolume > 0.1) {
        noiseVolume -= 0.001;
    }
    return true;
}

} // namespace Synth::Dummy
