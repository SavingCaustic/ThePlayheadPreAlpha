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

namespace Synth::Dummy {

enum class FilterType {
    LPF, // Low Pass Filter
    BPF, // Band Pass Filter
    HPF  // High Pass Filter
};

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

    float noiseVolume = 0.4;
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

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(int ccNumber, float value);
    //
    void setupParams();
};

} // namespace Synth::Dummy
