#pragma once

#include "Synth/SynthInterface.h"
#include "Synth/SynthParamManager.h"
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

namespace Synth::Sketch {

enum UP {
    osc1_fmsens,
    osc1_senstrack,
    osc2_semi,
    osc2_oct,
    osc_mix,
    vca_attack,
    vca_decay,
    vca_sustain,
    vca_fade,
    vca_release,
    vcf_type,
    vcf_cutoff,
    vcf_resonance,
    lfo1_depth,
    lfo1_vca,
    lfo1_speed,
    up_count
};

class Voice;
class Model : public SynthParamManager, public SynthInterface {

  public:
    // Constructor
    Model();
    // Public methods. These should match interface right (contract)
    void reset() override;
    void bindBuffers(float *audioBuffer, std::size_t bufferSize);

    nlohmann::json getParamDefsAsJSON() override {
        return SynthParamManager::getParamDefsAsJson();
    }

    void pushStrParam(const std::string &name, float val) override {
        return SynthParamManager::pushStrParam(name, val);
    }

    // Method to parse MIDI commands
    void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) override;

    // Method to render the next block of audio
    bool renderNextBlock() override;

    void addToSample(std::size_t sampleIdx, float val);
    float getOscMix();

    const audio::osc::LUT &getLUT1() const;
    const audio::osc::LUT &getLUT2() const;

    float oscMix = 0.5;
    int semitone = 0;
    int osc2octave = 0;
    float bendCents = 0;
    float senseTracking = 0.0f;
    audio::envelope::ADSFR vcaAR;
    float *buffer; // Pointer to audio buffer, minimize write so:
    float synthBuffer[TPH_RACK_BUFFER_SIZE];
    float fmSens = 0.0f;
    audio::osc::LUT lut1;
    audio::osc::LUT lut2;
    std::size_t bufferSize; // Size of the audio buffer

  protected:
    std::vector<Voice> voices; // Vector to hold Voice objects
    audio::filter::MultiFilter filter;
    audio::lfo::RampLfo lfo1;
    audio::lfo::SimpleLfo lfo2;

    // float vcaEaser,
    // vcaEaserStep;

    // Parameter definitiongs
    // std::unordered_map<std::string, ParamDefinition> parameterDefinitions;

    // CC mapping
    // std::unordered_map<int, std::string> ccMappings;

    // Private methods
    // void setupCCmapping(const std::string &path);

    // void renderVoice();
    int8_t findVoiceToAllocate(uint8_t note);

    void motherboardActions();

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(uint8_t ccNumber, float value);
    //
    void setupParams(int upCount);
    int debugCount = 0;
    float lfo1Depth = 0.5;
    float lfo1vca = 0.0f;
};

// ---------------------------------------
// VOICE - could be moved to separate file
// ---------------------------------------

class Voice {
    // pass reference to model so we can use AR there. The voice has the ARslope.
  public:
    // Constructor initializes modelRef and LUTs for oscillators
    Voice(Model &model);

    void reset();

    void noteOn(uint8_t midiNote, float velocity);

    void noteOff();

    audio::envelope::ADSFRState getVCAstate() {
        return vcaARslope.state;
    }

    float getVCALevel();

    bool checkVoiceActive();

    bool renderNextVoiceBlock(std::size_t bufferSize);

    uint8_t notePlaying;
    audio::envelope::Slope vcaARslope;

  protected:
    audio::osc::LUTosc osc1;
    audio::osc::LUTosc osc2;
    audio::misc::Easer oscMixEaser;
    audio::misc::Easer vcaEaser;
    float noteVelocity;
    float velocityLast = 0;
    float vcaEaserVal;
    float tracking;
    // float voiceBuffer[TPH_RACK_RENDER_SIZE];

  private:
    Model &modelRef; // Use reference to Model
    float oscMixEaseOut = 0;
    float fmAmpEaseOut = 0;
};

} // namespace Synth::Sketch
