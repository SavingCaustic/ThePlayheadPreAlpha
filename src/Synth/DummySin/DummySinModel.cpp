#include "./DummySinModel.h"
#include <cmath>
#include <iostream>
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
    if (SynthInterface::parameterDefs.empty()) {
        SynthInterface::parameterDefs = {
            {"pan", {0.1f, 0, false, 0, 1, [this](float v) {
                         gainLeft = AudioMath::ccos(v * 0.25f);  // Left gain decreases as panVal goes to 1
                         gainRight = AudioMath::csin(v * 0.25f); // Right gain increases as panVal goes to 1
                     }}},
            {"cutoff", {0.5f, 0, true, 50, 8, [this](float v) {
                            std::cout << "Setting cutoff to " << v << std::endl;
                            cutoffHz = v;
                            initFilter();
                        }}},
            {"detune", {0.5f, 0, false, -100, 100, [this](float v) {
                            std::cout << "Setting detune to " << v << std::endl;
                        }}},
            {"filter_type", {0.0f, 3, false, 0, 2, [this](float v) {
                                 int iv = static_cast<int>(v);
                                 switch (iv) {
                                 case 0:
                                     filterType = FilterType::LPF;
                                     break;
                                 case 1:
                                     filterType = FilterType::BPF;
                                     break;
                                 case 2:
                                     filterType = FilterType::HPF;
                                     break;
                                 }
                                 initFilter();
                             }}},
            {"semitone", {0.5f, 13, false, -6, 6, [this](float v) {
                              std::cout << "Setting semitone to " << v << std::endl;
                              semitone = round(v);
                          }}}};
    }
}

void Model::reset() {
    this->applySine(1, 0.5);
    this->applySine(2, 0.4);
    this->applySine(3, 0.3);
    this->applySine(4, 0.2);
    this->applySine(5, 0.1);
    this->applySine(6, 0.1);
    this->applySine(7, 0.3);
    AudioMath::normalizeLUT(lut1, LUT_SIZE);
}

void Model::parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) {
    uint8_t messageType = cmd & 0xf0;
    float fParam2 = static_cast<float>(param2) * (1.0f / 127.0f);
    switch (messageType) {
    case 0x80:
        if (param1 == notePlaying)
            vcaLevelTarget = 0.0f;
        break;
    case 0x90:
        notePlaying = param1;
        vcaLevelTarget = 1.0f;
        break;
    case 0xb0:
        SynthInterface::handleMidiCC(param1, fParam2);
        break;
    case 0xe0: {
        int pitchBendValue = (static_cast<int>(param2) << 7) | static_cast<int>(param1);
        bendCents = ((pitchBendValue - 8192) / 8192.0f) * 200.0f;
        break;
    }
    }
}

void Model::applySine(float multiple, float amplitude) {
    for (size_t i = 0; i < LUT_SIZE; ++i) {
        lut1[i] += std::sin(2.0f * M_PI * i * multiple / LUT_SIZE) * amplitude;
    }
}

void Model::initFilter() {
    // also called on cutoff-change
    float RC, RC2;
    float dt = 1.0f / TPH_DSP_SR;
    std::cout << "Initializing filter with cutoff frequency: " << cutoffHz << std::endl;

    switch (filterType) {
    case FilterType::LPF:
        RC = 1.0f / (2.0f * M_PI * cutoffHz);
        RC2 = 1.0f / (2.0f * M_PI * (cutoffHz * 1.5f)); // Example for bandwidth
        alpha = dt / (RC + dt);                         // Low-pass alpha
        alpha2 = dt / (RC2 + dt);                       // High-pass alpha
        break;

    case FilterType::BPF:
        RC = 1.0f / (2.0f * M_PI * cutoffHz);
        RC2 = 1.0f / (2.0f * M_PI * (cutoffHz * 1.5f)); // Example for bandwidth
        alpha = dt / (RC + dt);
        std::cout << "BPF alpha: " << alpha << std::endl;
        break;

    case FilterType::HPF:
        RC = 1.0f / (2.0f * M_PI * cutoffHz);
        alpha = RC / (RC + dt);
        std::cout << "HPF alpha: " << alpha << std::endl;
        break;
    }
}

bool Model::renderNextBlock() {
    float hz = AudioMath::noteToHz(notePlaying + semitone, bendCents);
    angle = hz * LUT_SIZE / TPH_DSP_SR;

    for (std::size_t i = 0; i < bufferSize; i++) {
        lutIdx = (lutIdx + angle) < LUT_SIZE ? (lutIdx + angle) : (lutIdx + angle - LUT_SIZE);
        int intIdx = static_cast<int>(lutIdx);
        float y = lut1[intIdx] * 0.5f * vcaLevel;

        buffer[i] = y * gainLeft;

        vcaLevel += vcaLevelDelta;
        applyFilter(buffer[i]);
    }

    vcaLevelDelta = (vcaLevelTarget - vcaLevel) * 0.015f;

    return false; // mono
}

void Model::applyFilter(float &sample) {
    switch (filterType) {
    case FilterType::LPF:
        sample = alpha * sample + (1.0f - alpha) * previousLeft;
        previousLeft = sample;
        break;
    case FilterType::BPF:
        // High-pass stage
        highPassedSample = alpha2 * (sample - previousLeft) + (1 - alpha2) * highPassedSample;
        previousLeft = sample; // Store the current sample for the next iteration

        // Low-pass stage on the high-passed signal
        lowPassedSample = alpha * highPassedSample + (1 - alpha) * lowPassedSample;

        // Output the band-passed sample
        sample = lowPassedSample;
        break;
    case FilterType::HPF:
        highPassedSample = alpha * (sample - previousLeft + highPassedSample);
        previousLeft = sample; // Store the current unfiltered sample for the next iteration
        sample = highPassedSample;
        break;
    }
}

} // namespace Synth::DummySin
