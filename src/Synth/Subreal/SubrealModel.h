#pragma once
#include "Synth/SynthBase.h"
#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/envelope/ASR.h"
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

namespace Synth::Subreal {

class VoiceInterface {
    /* not possible to use interface when voices are allocated statically */
  public:
    // Constructor initializes modelRef and LUTs for oscillators
    virtual ~VoiceInterface() = default;
    virtual void noteOn(uint8_t midiNote, float velocity) = 0;
    virtual void noteOff() = 0;
    virtual audio::envelope::ADSFRState getVCAstate() = 0;
    virtual float getVCAlevel() = 0;
    virtual bool checkVoiceActive() = 0;
    virtual bool renderNextVoiceBlock(std::size_t bufferSize) = 0;

    uint8_t notePlaying;

  protected:
    VoiceInterface() = default; // Allow only derived classes to instantiate
};

enum UP {
    osc1_wf,
    osc1_fmsens,
    osc1_amsens,
    osc1_senstrack,
    osc2_semi,
    osc2_oct,
    osc2_noise_mix,
    osc2_freqtrack,
    osc_mix,
    osc_detune,
    vcf_cutoff,
    vcf_resonance,
    vcf_type,
    vcf_shape,
    vcf_attack,
    vcf_decay,
    vcf_sustain,
    vcf_fade,
    vcf_release,
    vca_attack,
    vca_decay,
    vca_sustain,
    vca_fade,
    vca_release,
    vca_spatial,
    lfo1_shape,
    lfo1_speed,
    lfo1_routing,
    lfo1_depth,
    lfo2_shape,
    lfo2_speed,
    lfo2_routing,
    lfo2_depth,
    peg_atime,
    peg_rtime,
    peg_asemis,
    peg_rsems,
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
    void initSettings();
    // void updateSetting(std::string key, std::string value);
    // void updateSetting(const std::string &key, const std::string &value, void *object, uint32_t size, bool isStereo);
    void updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Constructor::Queue &constructorQueue) override;

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
    audio::envelope::ADSFR vcfAR;
    audio::envelope::ASR pegAR;
    float *buffer; // Pointer to audio buffer, minimize write so:
    float synthBuffer[TPH_RACK_BUFFER_SIZE];
    float fmSens = 0.0f;
    LFO1::Routing lfo1Routing;
    LFO2::Routing lfo2Routing;
    audio::osc::LUT *lut1 = nullptr;
    audio::osc::LUT *lut2 = nullptr;
    std::size_t bufferSize; // Size of the audio buffer
    float lfo1depth = 0.5;
    float lfo2depth = 0.5;
    audio::lfo::Standard lfo1; // change to ramp. duh. no, it's in voice..
    audio::lfo::Standard lfo2;
    float vcaSpatial;
    float osc2noiseMix;
    float amSens;
    float filterCutoff = 500;
    float filterResonance = 0.5;
    bool vcfInverse = false;
    audio::filter::FilterType filterType = audio::filter::FilterType::highPass;
    audio::filter::FilterPoles filterPoles = audio::filter::FilterPoles::p2;

  private:
    void buildLUT(audio::osc::LUT &lut, const std::string val);

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

class Voice { //: public VoiceInterface {
              // pass reference to model so we can use AR there. The voice has the ARslope.
  public:
    // Constructor initializes modelRef and LUTs for oscillators
    Voice(Model &model)
        : modelRef(model),
          osc1(), // Initialize osc1 with LUT from model
          osc2()  // Initialize osc2 with LUT from model
    {
        reset();
    }

    void reset() {
        // Called on setup
        notePlaying = 255; // unsigned byte. best like this..
    }

    void setLUTs(const audio::osc::LUT &lut1, const audio::osc::LUT &lut2) {
        osc1.setLUT(lut1); // Set LUT for osc1
        osc2.setLUT(lut2); // Set LUT for osc2
    }

    void noteOn(uint8_t midiNote, float velocity) {
        notePlaying = midiNote;
        // setup stuff based on note-value. But not frequency.
        leftAtt = fmin(1, fmax(0, (notePlaying - 60) * 0.04f * modelRef.vcaSpatial));
        rightAtt = fmin(1, fmax(0, (60 - notePlaying) * 0.04f * modelRef.vcaSpatial));
        noteVelocity = velocity;
        mixAmplitude = noteVelocity * 0.4f; // Factor to avoid too much dist on polyphony..
        tracking = fmax(0, (2.0f + modelRef.senseTracking * AudioMath::noteToFloat(notePlaying) * 7));
        modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_ON);
        modelRef.vcfAR.triggerSlope(vcfARslope, audio::envelope::NOTE_ON);
        filter.initFilter();
    }

    void noteOff() {
        // enter release state in all envelopes.
        modelRef.vcaAR.triggerSlope(vcaARslope, audio::envelope::NOTE_OFF);
        modelRef.vcfAR.triggerSlope(vcfARslope, audio::envelope::NOTE_OFF);
    }

    audio::envelope::ADSFRState getVCAstate() {
        return vcaARslope.state;
    }

    float getVCAlevel() {
        return vcaARslope.currVal;
    }

    bool checkVoiceActive() {
        return vcaARslope.state != audio::envelope::ADSFRState::OFF;
    }

    bool renderNextVoiceBlock(std::size_t bufferSize) {
        float osc1hz, osc2hz;
        // Chunk could be set to SIMD-capability.
        constexpr int chunkSize = 16;
        float chunkSample[chunkSize]; // cache-optimized storage for chunks. (mono)
        if (vcaARslope.state != audio::envelope::OFF) {
            // put stuff here that should be interactive on note on.
            // calc osc2
            int osc2note = notePlaying + modelRef.semitone + modelRef.osc2octave * 12;
            float osc2cents = modelRef.bendCents + modelRef.oscDetune;
            if (modelRef.lfo1Routing == LFO1::Routing::osc12 || modelRef.lfo1Routing == LFO1::Routing::osc2) {
                osc2cents += modelRef.lfo1.getLFOval() * modelRef.lfo1depth * 200.0f;
            }
            osc2hz = AudioMath::noteToHz(osc2note, osc2cents);
            osc2.setAngle(osc2hz);
            // calc fmSens
            float fmAmp = tracking * modelRef.fmSens * noteVelocity;
            if (modelRef.lfo2Routing == LFO2::Routing::fmSens) {
                fmAmp *= (1 + modelRef.lfo2.getLFOval() * modelRef.lfo2depth);
            }
            // calc note and cent for osc1
            float osc1cents = modelRef.bendCents - modelRef.oscDetune;
            if (modelRef.lfo1Routing == LFO1::Routing::osc12 || modelRef.lfo1Routing == LFO1::Routing::osc1) {
                osc1cents += modelRef.lfo1.getLFOval() * modelRef.lfo1depth * 200.0f;
            }
            osc1hz = AudioMath::noteToHz(notePlaying, osc1cents);
            osc1.setAngle(osc1hz);
            // setup new delta (lin-easer) for VCA
            modelRef.vcaAR.updateDelta(vcaARslope);
            float vcaTarget = vcaARslope.currVal + vcaARslope.gap;
            // same for VCF
            modelRef.vcfAR.updateDelta(vcfARslope);
            float vcfTarget = vcfARslope.currVal + vcfARslope.gap;

            if (modelRef.lfo1Routing == LFO1::Routing::vca) {
                vcaTarget *= (modelRef.lfo1.getLFOval() * modelRef.lfo1depth) + 1.0f;
            }
            if (modelRef.lfo2Routing == LFO2::Routing::vca) {
                vcaTarget *= (modelRef.lfo2.getLFOval() * modelRef.lfo2depth) + 1.0f;
            }
            vcaEaser.setTarget(vcaTarget);
            vcfEaser.setTarget(vcfTarget);

            for (std::size_t i = 0; i < bufferSize; i += chunkSize * 2) {
                // chunk here.. 1/2/3. Oscillators | Filter | VCA
                AudioMath::easeLog50(fmAmp, fmAmpEaseOut);
                AudioMath::easeLog5(modelRef.oscMix, oscMixEaseOut);
                for (std::size_t j = 0; j < chunkSize; j++) {
                    chunkSample[j] = osc2.getNextSample(0);
                }
                if (modelRef.osc2noiseMix > 0.02f) {
                    for (std::size_t j = 0; j < chunkSize; j++) {
                        float noise = AudioMath::noise();
                        chunkSample[j] = chunkSample[j] * (1.0f - modelRef.osc2noiseMix) + noise * modelRef.osc2noiseMix;
                    }
                }
                for (std::size_t j = 0; j < chunkSize; j++) {
                    float y1 = osc1.getNextSample(chunkSample[j] * fmAmpEaseOut);
                    chunkSample[j] = y1 * (1 - oscMixEaseOut) + chunkSample[j] * oscMixEaseOut;
                }
                // VCF (100Hz is appropirate for lpf - maybe not for others..)
                if (modelRef.vcfInverse) {
                    filter.setCutoff(100 + modelRef.filterCutoff * (1.0f - vcfARslope.currVal));
                } else {
                    filter.setCutoff(100 + modelRef.filterCutoff * vcfARslope.currVal);
                }
                filter.setResonance(modelRef.filterResonance);
                filter.setPoles(modelRef.filterPoles);
                filter.setFilterType(modelRef.filterType);
                filter.initFilter();
                filter.processBlock(chunkSample, chunkSize);
                // VCA
                for (std::size_t j = 0; j < chunkSize; j++) {
                    vcaEaserVal = vcaEaser.getValue();
                    chunkSample[j] = chunkSample[j] * vcaEaserVal * mixAmplitude;
                }
                // send to model (sum)
                for (std::size_t j = 0; j < chunkSize; j++) {
                    modelRef.addToSample(i + j * 2, chunkSample[j] * (1 - leftAtt));
                    modelRef.addToSample(i + j * 2 + 1, chunkSample[j] * (1 - rightAtt));
                }
            }
            // copy local voice-buffer to synth-buffer
            modelRef.vcaAR.commit(vcaARslope);
            modelRef.vcfAR.commit(vcfARslope);
        } else {
            // Remove voice from playing
            // maybe use return-type
        }
        return true;
    }
    uint8_t notePlaying;

  protected:
    audio::osc::LUTosc osc1; // First oscillator (not a pointer)
    audio::osc::LUTosc osc2; // Second oscillator (not a pointer)
    audio::filter::MultiFilter filter;
    audio::envelope::ADSFRSlope vcaARslope;
    audio::envelope::ADSFRSlope vcfARslope; // should prob be called vcfEnvSlope..
    // audio::misc::Easer oscMixEaser;
    audio::misc::Easer vcaEaser; // maybe replace with audioMath-inline easeLin(delta,target)
    audio::misc::Easer vcfEaser; // maybe replace with audioMath-inline easeLin(delta,target)
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
    float mixAmplitude;
};

} // namespace Synth::Subreal
