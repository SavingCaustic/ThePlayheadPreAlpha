#pragma once

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
class Model : public SynthInterface {

  public:
    // Constructor
    Model(float *audioBuffer, std::size_t bufferSize);
    // Public methods. These should match interface right (contract)
    void reset() override;

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
    Voice(Model &model)
        : modelRef(model),
          osc1(model.getLUT1()), // Initialize osc1 with LUT from model
          osc2(model.getLUT2())  // Initialize osc2 with LUT from model
    {
        reset();
    }

    void reset() {
        // Called on setup
        notePlaying = 255; // unsigned byte. best like this..
    }

    void noteOn(uint8_t midiNote, float velocity) {
        // requested from voiceAllocate. maybe refactor..
        notePlaying = midiNote;
        noteVelocity = velocity;
        tracking = fmax(0, (2.0f + modelRef.senseTracking * AudioMath::noteToFloat(notePlaying) * 7));
        modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_ON);
    }

    void noteOff() {
        // enter release state in all envelopes.
        modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_OFF);
    }

    audio::envelope::ADSFRState getVCAstate() {
        return vcaARslope.state;
    }

    float getVCALevel() {
        return vcaARslope.currVal;
    }

    bool checkVoiceActive() {
        return vcaARslope.state != audio::envelope::ADSFRState::OFF;
    }

    bool renderNextVoiceBlock(std::size_t bufferSize) {
        float osc1hz, osc2hz;
        constexpr int chunkSize = 16; // Could be adapted to SIMD-capability..
        float oscMix = modelRef.getOscMix();
        modelRef.vcaAR.updateDelta(vcaARslope);
        float fmAmplitude = tracking * modelRef.fmSens * noteVelocity;
        float mixAmplitude = noteVelocity * 0.6f;
        if (vcaARslope.state != audio::envelope::OFF) {
            osc2hz = AudioMath::noteToHz(notePlaying + modelRef.semitone + modelRef.osc2octave * 12, modelRef.bendCents);
            osc2.setAngle(osc2hz);
            osc1hz = AudioMath::noteToHz(notePlaying, modelRef.bendCents);
            osc1.setAngle(osc1hz);
            vcaEaser.setTarget(vcaARslope.currVal + vcaARslope.gap);
            AudioMath::easeLog2(oscMix, oscMixEaseOut);
            for (std::size_t i = 0; i < bufferSize; i += chunkSize) {
                for (std::size_t j = 0; j < chunkSize; j++) {
                    float y2 = osc2.getNextSample(0);
                    vcaEaserVal = vcaEaser.getValue();
                    float y1 = osc1.getNextSample(y2 * fmAmplitude * (vcaEaserVal + 0.3f));
                    modelRef.addToSample(i + j, ((y1 * (1 - oscMixEaseOut) + y2 * oscMixEaseOut)) * vcaEaserVal * mixAmplitude);
                }
            }
            modelRef.vcaAR.commit(vcaARslope);
        } else {
            // Remove voice from playing
            // maybe use return-type
        }
        return true;
    }

    uint8_t notePlaying;

  protected:
    audio::osc::LUTosc osc1;
    audio::osc::LUTosc osc2;
    audio::envelope::Slope vcaARslope;
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
};

} // namespace Synth::Sketch
