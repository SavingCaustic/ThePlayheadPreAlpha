#pragma once
#include "Synth/SynthBase.h"
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

using json = nlohmann::json;

namespace Synth::Beatnik {

enum UP {
    osc1_wf,
    osc1_fmsens,
    osc1_amsens,
    osc1_senstrack,
    osc2_semi,
    osc2_oct,
    osc2_freqtrack,
    osc_mix,
    osc_detune,
    vca_attack,
    vca_decay,
    vca_sustain,
    vca_fade,
    vca_release,
    vca_spatial,
    vcf_type,
    vcf_cutoff,
    vcf_resonance,
    lfo1_shape,
    lfo1_speed,
    lfo1_routing,
    lfo1_depth,
    lfo2_shape,
    lfo2_speed,
    lfo2_routing,
    lfo2_depth,
    up_count
};

namespace LFO1 {
enum Routing {
    off,
    osc1,
    osc2,
    osc12,
    vcf,
    vca,
    _count
};
}

namespace LFO2 {
enum Routing {
    off,
    osc2,
    vcf,
    vca,
    fmSens,
    amSens,
    _count
};
}

namespace OSC1 {
enum WF {
    sine,
    tri,
    square,
    saw,
    noise,
    lut1,
    lut2,
    _count
};
}

namespace OSC2 {
enum class WF {
    sine,
    tri,
    square,
    saw,
    noise,
    lut1,
    lut2,
    _count
};
}

class Voice;
class Model : public SynthBase {

  public:
    // Constructor
    Model();
    // Public methods. These should match interface right (contract)
    void reset() override;
    void bindBuffers(float *audioBuffer, std::size_t bufferSize);

    json getParamDefsAsJSON() override {
        return SynthBase::getParamDefsAsJson();
    }

    void pushStrParam(const std::string &name, float val) override {
        return SynthBase::pushStrParam(name, val);
    }

    // Method to parse MIDI commands
    void parseMidi(uint8_t cmd, uint8_t param1, uint8_t param2) override;

    // Method to render the next block of audio
    bool renderNextBlock() override;

    void addToSample(std::size_t sampleIdx, float val);
    float getOscMix();

    const audio::osc::LUT &getLUT1() const;
    const audio::osc::LUT &getLUT2() const;

    float oscMix = 0.5f;
    float oscDetune = 0.0f;
    float osc2freqTrack;
    int semitone = 0;
    int osc2octave = 0;
    float bendCents = 0;
    float senseTracking = 0.0f;
    audio::envelope::ADSFR vcaAR;
    float *buffer; // Pointer to audio buffer, minimize write so:
    float synthBuffer[TPH_RACK_BUFFER_SIZE];
    float fmSens = 0.0f;
    LFO1::Routing lfo1Routing;
    LFO2::Routing lfo2Routing;
    audio::osc::LUT lut1;
    audio::osc::LUT lut2;
    std::size_t bufferSize; // Size of the audio buffer
    float lfo1depth = 0.5;
    float lfo2depth = 0.5;
    audio::lfo::Standard lfo1; // change to ramp. duh. no, it's in voice..
    audio::lfo::Standard lfo2;
    audio::filter::MultiFilter filter;
    float vcaSpatial;
    float amSens;

  protected:
    std::vector<Voice> voices; // Vector to hold Voice objects

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
    // void handleMidiCC(uint8_t ccNumber, float value);
    //
    void setupParams(int upCount);
    int debugCount = 0;
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
        notePlaying = midiNote;
        // setup stuff based on note-value. But not frequency.
        leftAtt = fmin(1, fmax(0, (notePlaying - 60) * 0.04f * modelRef.vcaSpatial));
        rightAtt = fmin(1, fmax(0, (60 - notePlaying) * 0.04f * modelRef.vcaSpatial));
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
        // Chunk could be set to SIMD-capability.
        constexpr int chunkSize = 16;
        if (vcaARslope.state != audio::envelope::OFF) {
            // put stuff here that should be interactive on note on.
            float fmAmp = tracking * modelRef.fmSens * noteVelocity;
            if (modelRef.lfo2Routing == LFO2::Routing::fmSens) {
                fmAmp *= (1 + modelRef.lfo2.getLFOval() * modelRef.lfo2depth);
            }
            AudioMath::easeLog50(fmAmp, fmAmpEaseOut);
            float mixAmplitude = noteVelocity * 0.6f;
            AudioMath::easeLog5(modelRef.oscMix, oscMixEaseOut);
            int osc2note = notePlaying + modelRef.semitone + modelRef.osc2octave * 12;
            float osc2cents = modelRef.bendCents + modelRef.oscDetune;
            if (modelRef.lfo1Routing == LFO1::Routing::osc12 || modelRef.lfo1Routing == LFO1::Routing::osc2) {
                osc2cents += modelRef.lfo1.getLFOval() * modelRef.lfo1depth * 1200.0f;
            }
            osc2hz = AudioMath::noteToHz(osc2note, osc2cents);
            osc2.setAngle(osc2hz);
            osc1hz = AudioMath::noteToHz(notePlaying, modelRef.bendCents - modelRef.oscDetune);
            osc1.setAngle(osc1hz);
            // setup new delta (lin-easer) for VCA
            modelRef.vcaAR.updateDelta(vcaARslope);
            float vcaTarget = vcaARslope.currVal + vcaARslope.gap;
            if (modelRef.lfo1Routing == LFO1::Routing::vca) {
                vcaTarget *= (modelRef.lfo1.getLFOval() * modelRef.lfo1depth) + 1.0f;
            }
            if (modelRef.lfo2Routing == LFO2::Routing::vca) {
                vcaTarget *= (modelRef.lfo2.getLFOval() * modelRef.lfo2depth) + 1.0f;
            }
            vcaEaser.setTarget(vcaTarget);

            for (std::size_t i = 0; i < bufferSize; i += chunkSize) {
                for (std::size_t j = 0; j < chunkSize; j = j + 2) {
                    float y2 = osc2.getNextSample(0);
                    vcaEaserVal = vcaEaser.getValue();
                    float y1 = osc1.getNextSample(y2 * fmAmp * (vcaEaserVal + 0.3f)) * (1.0f + modelRef.amSens * y2);
                    // now weight sample to channel based on noteval.
                    float voiceOut = ((y1 * (1 - oscMixEaseOut) + y2 * oscMixEaseOut)) * vcaEaserVal * mixAmplitude;
                    modelRef.addToSample(i + j, voiceOut * (1 - leftAtt));
                    modelRef.addToSample(i + j + 1, voiceOut * (1 - rightAtt));
                }
            }
            // copy local voice-buffer to synth-buffer
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
    float leftAtt;
    float rightAtt;
    float noteVelocity;
    float velocityLast = 0;
    float vcaEaserVal;
    float tracking;

  private:
    Model &modelRef; // Use reference to Model
    float oscMixEaseOut = 0;
    float fmAmpEaseOut = 0;
};

} // namespace Synth::Beatnik
