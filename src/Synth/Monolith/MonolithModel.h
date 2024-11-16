#pragma once

// Forward declare Rack to avoid circular dependency
// class Rack;

#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
#include "core/audio/lfo/LFO.h"
#include "core/audio/misc/Easer.h"
#include "core/audio/osc/LUT.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Synth::Monolith {

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

    audio::osc::LUT lut1;
    audio::osc::LUT lut2;
    audio::osc::LUTosc osc1;
    audio::osc::LUTosc osc2;
    audio::filter::MultiFilter filter;
    audio::envelope::ADSFR vcaAR;
    audio::envelope::Slope vcaARslope;
    audio::lfo::RampLfo lfo1;
    audio::lfo::SimpleLfo lfo2;
    audio::misc::Easer oscMixEaser;
    audio::misc::Easer vcaEaser;
    float velocityLast = 0; // super-easy easer
    float vcaEaserVal;

    // float vcaEaser,
    // vcaEaserStep;

    // Parameter definitiongs
    // std::unordered_map<std::string, ParamDefinition> parameterDefinitions;

    // CC mapping
    // std::unordered_map<int, std::string> ccMappings;

    // Private methods
    // void setupCCmapping(const std::string &path);

    void initFilter();

    void applyFilter(float &sample);

    void applySine(float multiple, float amplitude);

    // void renderVoice();

    void motherboardActions();

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(uint8_t ccNumber, float value);
    //
    int notePlaying = 0;    // 0 = no note
    float noteVelocity = 0; // 0-1;
    void setupParams();
    float bendCents = 0;
    int semitone = 0;
    int osc2octave = 0;
    int debugCount = 0;
    float lfo1Depth = 0.5;
    float fmSens = 0.0f;
    float senseTracking = 0.0f;
    float lfo1vca = 0.0f;
};

} // namespace Synth::Monolith
