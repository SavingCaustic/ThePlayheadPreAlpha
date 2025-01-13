#pragma once
#include "Synth/SynthBase.h"
#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
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
    a_pan,
    b_pan,
    c_pan,
    d_pan,
    up_count
};

class Voice; // forward declare
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

  protected:
    std::vector<Voice> voices; // Vector to hold Voice objects

    // void renderVoice();
    int8_t findVoiceToAllocate(uint8_t note);

    void motherboardActions();

    float *buffer; // Pointer to audio buffer, minimize write so:
    float synthBuffer[TPH_RACK_BUFFER_SIZE];

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
          osc1(), // Initialize osc1 with LUT from model
          osc2()  // Initialize osc2 with LUT from model
    {
        reset();
    }

    void reset() {
        // Called on setup
        notePlaying = 255; // unsigned byte. best like this..
    }

    void noteOn(uint8_t midiNote, float velocity) {
        notePlaying = midiNote;
        leftAtt = 1.0f;
        rightAtt = 1.0f;
        noteVelocity = velocity;
        // it's all in the voice. no AR in model.
        vcaAR.triggerSlope(vcaARslope, audio::envelope::ADSFRCmd::NOTE_ON);
    }

    void noteOff() {
        // enter release state in all envelopes.
        vcaAR.triggerSlope(vcaARslope, audio::envelope::ADSFRCmd::NOTE_OFF);
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
        constexpr int chunkSize = 16;
        if (vcaARslope.state != audio::envelope::ADSFRState::OFF) {
            for (std::size_t i = 0; i < bufferSize; i += chunkSize) {
                for (std::size_t j = 0; j < chunkSize; j = j + 2) {
                    float y2 = osc2.getNextSample(0);
                    float voiceOut = y2;
                    modelRef.addToSample(i + j, voiceOut * (1 - leftAtt));
                    modelRef.addToSample(i + j + 1, voiceOut * (1 - rightAtt));
                }
            }
            // copy local voice-buffer to synth-buffer
            vcaAR.commit(vcaARslope);
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
    audio::envelope::ADSFR vcaAR;
    audio::envelope::ADSFRSlope vcaARslope;
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
