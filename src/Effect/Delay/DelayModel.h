#pragma once

// Forward declare Rack to avoid circular dependency
// class Rack;

#include "Effect/EffectInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/params.h"
#include <array>
#include <cstdlib> // for rand()
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Effect::Delay {

class Model : public EffectInterface {
  public:
    // Constructor
    Model(float *audioBuffer, std::size_t bufferSize);
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to parse MIDI commands
    void parseMidi(char cmd, char param1, char param2) override;
    bool pushMyParam(const std::string &name, float val);

    // Method to render the next block of audio
    bool renderNextBlock() override;

  protected:
    float *buffer;          // Pointer to audio buffer
    std::size_t bufferSize; // Size of the audio buffer
    std::vector<float> delayBuffer;
    int wrPointer = 0;
    int rdPointer = 0;
    float mix = 0.3;
    float feedback = 0.05;
    float time = 0.6;

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(int ccNumber, float value);
    //
    void setupParams();
};

} // namespace Effect::Delay
