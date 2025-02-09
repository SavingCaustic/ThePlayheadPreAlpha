#pragma once

// Forward declare Rack to avoid circular dependency
// class Rack;

#include "Effect/EffectBase.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib> // for rand()
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Effect::Delay {
using json = nlohmann::json;

enum UP {
    time,
    mix,
    feedback,
    highcut,
    noise,
    lowcut,
    up_count
};

// moved outside of lambda to avoid re-creation.
const constexpr uint8_t idx2clocks[11] = {2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64};

class Model : public EffectBase {
  public:
    // note that pointers are 16-bit unsigned so this is max (but mono)..
    static constexpr uint32_t BUFFER_SIZE = 65536;

    // Constructor
    Model();
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to render the next block of audio
    bool renderNextBlock(bool isSterero) override;

    json getParamDefsAsJSON() override {
        return EffectBase::getParamDefsAsJson();
    }

    void processClock(const uint8_t clock24) override;

  protected:
    // oh this should be a pointer, set up offline..
    // well actually - if there's a factory, the whoule effect is setup so keep design.
    std::vector<float> delayBufferLeft;
    std::vector<float> delayBufferRight; // may be stupid empty
    float iBuffer[TPH_RACK_RENDER_SIZE];
    uint16_t wrPointer; // set by reset
    uint16_t rdPointer;
    uint8_t delayInClocks;
    int clockSampleGap = 0;
    int last8thWritePos = 0;

    int eightsSampleGap = 2000; // trigger change on first call

    //??
    uint16_t delayInSamples = 100;
    //
    int sampleGap = 0;
    float sampleGapEaseOut = 0;
    float mix = 0.3;
    float feedback = 0.2f;
    float time = 0.57f; // 105 bpm
    float lowcut = 0;
    float highcut = 0;
    float noise = 0; // noise is filtered too, for pink noise.
    int debugCnt = 0;

    // temp:
    bool clockTick = false;

    void initializeParameters();
    //
    void setupParams(int upCount);

  private:
    float highCutLeft, highCutRight;
    float lowCutLeft, lowCutRight;
    float noisePink;
    double RMS;
};

} // namespace Effect::Delay
