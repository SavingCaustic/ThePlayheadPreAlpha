#pragma once
#include "BeatnikVoice.h"
#include "Synth/SynthBase.h"
#include "Synth/SynthInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/filter/MultiFilter.h"
#include "core/audio/misc/Easer.h"
#include "core/audio/osc/LUT.h"
#include "core/audio/sample/SimpleSample.h"
#include "core/destructor/Queue.h"
#include "core/parameters/params.h"
#include <array>
#include <cmath>
#include <core/utils/FNV.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

namespace Synth::Beatnik {

enum UP {
    a_pan,
    b_pan,
    c_pan,
    d_pan,
    up_count
};

class Model : public SynthBase {

  public:
    // Constructor
    Model();
    // Public methods. These should match interface right (contract)
    void reset() override;
    void bindBuffers(float *audioBufferLeft, float *audioBufferRight, std::size_t bufferSize);

    void updateSetting(const std::string &type, void *object, uint32_t size, bool isStereo, Destructor::Record &recordDelete) override; //, Constructor::Queue &constructorQueue) override;

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

    void addToLeftSample(std::size_t sampleIdx, float val);
    void addToRightSample(std::size_t sampleIdx, float val);

  public:
    std::vector<Voice> voices; // Vector to hold Voice objects
    std::array<audio::sample::SimpleSample, 12> samples;
    // std::vector<audio::sample::SimpleSample> samples; // Vector ho hold sample objects

    // void renderVoice();
    int8_t findVoiceToAllocate(uint8_t note);

    void motherboardActions();

    float voiceBufferLeft[TPH_RACK_RENDER_SIZE];
    float voiceBufferRight[TPH_RACK_RENDER_SIZE];

    float *bufferLeft, *bufferRight; // Pointer to audio buffer, minimize write so:
    float synthBufferLeft[TPH_RACK_RENDER_SIZE], synthBufferRight[TPH_RACK_RENDER_SIZE];

    // void initializeParameters();
    //  Handle incoming MIDI CC messages
    //  void handleMidiCC(uint8_t ccNumber, float value);
    //
    void setupParams(int upCount);
    int debugCount = 0;
};

} // namespace Synth::Beatnik
