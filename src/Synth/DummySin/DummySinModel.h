#pragma once

// Forward declare Rack to avoid circular dependency
// class Rack;

#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib> // for rand()
#include <iostream>
#include <string>
#include <unordered_map>

struct Easer {
    float currentValue;
    float targetValue;
    float (*easingFunc)(float, float, float); // Easing function: input, target, time
    bool isActive;

    Easer() : currentValue(0.0f), targetValue(0.0f), easingFunc(nullptr), isActive(false) {}

    void update(float deltaTime) {
        if (isActive) {
            currentValue = easingFunc(currentValue, targetValue, deltaTime);
            if (fabs(currentValue - targetValue) < 0.001f) { // Threshold to stop easing
                isActive = false;
            }
        }
    }

    void start(float newTargetValue, float (*newEasingFunc)(float, float, float)) {
        targetValue = newTargetValue;
        easingFunc = newEasingFunc;
        isActive = true;
    }
};

namespace Synth::DummySin {

struct AR {
    float vol = 0;
    int state = 0;

    void noteOn() {
        state = 1;
    }

    void process() {
        if (state == 1) {
            if (vol < 1) {
                vol += (1 - vol) * 0.1;
            }
        }
    }
};

enum class FilterType {
    LPF, // Low Pass Filter
    BPF, // Band Pass Filter
    HPF  // High Pass Filter
};

// Artifacts at C1. constexpr int LUT_SIZE = 1024;
constexpr int LUT_SIZE = 4096;

class Model : public SynthInterface {

  public:
    // Constructor
    Model(float *audioBuffer, std::size_t bufferSize);
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to parse MIDI commands
    void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) override;
    bool pushMyParam(const std::string &name, float val);

    // Method to render the next block of audio
    bool renderNextBlock() override;

  protected:
    float *buffer;          // Pointer to audio buffer
    std::size_t bufferSize; // Size of the audio buffer

    float vcaLevelDelta = 0;
    float vcaLevel = 0.0;
    float vcaLevelTarget = 0;

    FilterType filterType; // New member variable for filter type

    // LPF variables
    float previousLeft = 0.0f;
    float previousRight = 0.0f;
    float alpha = 0.0f; // Filter coefficient
    int cutoffHz = 2000;
    float gainLeft = 0.7;
    float gainRight = 0.7;

    // Parameter definitiongs
    // std::unordered_map<std::string, ParamDefinition> parameterDefinitions;

    // CC mapping
    // std::unordered_map<int, std::string> ccMappings;

    // Private methods
    // void setupCCmapping(const std::string &path);

    void initLPF();
    void initBPF();
    void initHPF();

    void applySine(float multiple, float amplitude);

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(uint8_t ccNumber, float value);
    //
    int notePlaying = 0; // 0 = no note
    void setupParams();
    float lut1[LUT_SIZE]{};
    float lutIdx = 0;
    float angle = AudioMath::getMasterTune() * LUT_SIZE * (1.0f / TPH_DSP_SR);
    float bendCents = 0;
};

} // namespace Synth::DummySin
