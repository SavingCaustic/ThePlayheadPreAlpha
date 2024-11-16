#pragma once

#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
#include "core/audio/lfo/RampLfo.cpp"
#include "core/audio/lfo/SimpleLfo.cpp"
#include "core/audio/misc/Easer.h"
#include "core/audio/osc/LUT.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Synth::Subreal {
// using LUT = audio::osc::LUTosc;

//  Artifacts at C1. constexpr int LUT_SIZE = 1024;
constexpr int LUT_SIZE = 4096;

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

    const audio::osc::LUT &getLUT1() const;
    const audio::osc::LUT &getLUT2() const;

    int semitone = 0;
    int osc2octave = 0;
    float bendCents = 0;
    float senseTracking = 0.0f;
    audio::envelope::ADSFR vcaAR;
    float *buffer; // Pointer to audio buffer
    float fmSens = 0.0f;
    audio::osc::LUT lut1;
    audio::osc::LUT lut2;

  protected:
    std::size_t bufferSize; // Size of the audio buffer
    Voice sVoices[8];
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
    uint8_t findVoiceToAllocate(uint8_t note);

    void motherboardActions();

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(uint8_t ccNumber, float value);
    //
    int notePlaying = 0;    // 0 = no note
    float noteVelocity = 0; // 0-1;
    void setupParams();
    int debugCount = 0;
    float lfo1Depth = 0.5;
    float lfo1vca = 0.0f;
};

class Voice {
    // pass reference to model so we can use AR there. The voice has the ARslope.
  public:
    // Constructor initializes modelRef and LUTs for oscillators
    Voice(Model &model)
        : modelRef(model),
          osc1(modelRef.getLUT1()), // Initialize osc1 with LUT from model
          osc2(modelRef.getLUT2())  // Initialize osc2 with LUT from model
    {}

    void reset() {
        // Called on setup
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

    bool renderNextVoiceBlock() {
        float osc1hz, osc2hz;
        constexpr int chunkSize = 16;
        float osc1hzOld = -1;

        modelRef.vcaAR.updateDelta(vcaARslope);
        if (vcaARslope.state != audio::envelope::OFF) {
            osc2hz = AudioMath::noteToHz(notePlaying + modelRef.semitone + modelRef.osc2octave * 12, modelRef.bendCents);
            osc2.setAngle(osc2hz);
            vcaEaser.setTarget(vcaARslope.currVal + vcaARslope.gap);
            for (std::size_t i = 0; i < TPH_RACK_BUFFER_SIZE; i++) {
                float y2 = osc2.getNextSample(0);
                if (i % chunkSize == 0) {
                    osc1hz = AudioMath::noteToHz(notePlaying, modelRef.bendCents);
                    osc1hzOld += (osc1hz - osc1hzOld) / 100;
                    osc1.setAngle(osc1hz);
                }
                float tracking = fmax(0, (1.0f + modelRef.senseTracking * AudioMath::noteToFloat(notePlaying) * 5));
                float y1 = osc1.getNextSample(y2 * tracking * modelRef.fmSens * 2.0f * noteVelocity);

                velocityLast += (noteVelocity - velocityLast) / 10.0f;
                vcaEaserVal = vcaEaser.getValue();
                float oscMix = oscMixEaser.getValue();
                modelRef.buffer[i] += ((y1 * (1 - oscMix) + y2 * oscMix)) * vcaEaserVal * velocityLast * 2.0f;
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

  private:
    Model &modelRef; // Use reference to Model
};

} // namespace Synth::Subreal
