#pragma once

// Forward declare Rack to avoid circular dependency
// class Rack;

#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/AR.h"
#include "core/audio/misc/Easer.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib> // for rand()
#include <iostream>
#include <string>
#include <unordered_map>

namespace Synth::DummySin {

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
    float highPassedSample = 0.0f;
    float lowPassedSample = 0.0f;
    float previousRight = 0.0f;
    float alpha = 0.0f; // Filter coefficient
    float alpha2 = 0.0f;
    int cutoffHz = 2000;
    float gainLeft = 0.7;
    float gainRight = 0.7;

    audio::envelope::AR vcaAR;
    audio::envelope::ARState vcaARstate;
    audio::misc::Easer vcaEaser;

    // Parameter definitiongs
    // std::unordered_map<std::string, ParamDefinition> parameterDefinitions;

    // CC mapping
    // std::unordered_map<int, std::string> ccMappings;

    // Private methods
    // void setupCCmapping(const std::string &path);

    void initFilter();

    void applyFilter(float &sample);

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
    int semitone = 0;
    int debugCount = 0;
};

} // namespace Synth::DummySin
