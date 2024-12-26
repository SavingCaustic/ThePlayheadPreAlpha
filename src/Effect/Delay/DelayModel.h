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
    lowcut,
    noise,
    up_count
};

class Model : public EffectBase {
  public:
    static constexpr uint32_t BUFFER_SIZE = 1024 * 128;

    // Constructor
    Model();
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to render the next block of audio
    bool renderNextBlock(bool isSterero) override;

    json getParamDefsAsJSON() override {
        return EffectBase::getParamDefsAsJson();
    }

  protected:
    // oh this should be a pointer, set up offline..
    // well actually - if there's a factory, the whoule effect is setup so keep design.
    std::vector<float> delayBuffer;
    int wrPointer = 0;
    int sampleGap = 0;
    float sampleGapEaseOut = 0;
    int rdPointer = 0;
    float mix = 0.3;
    float feedback = 0.2f;
    float time = 0.57f; // 105 bpm
    float lowcut = 0;
    float highcut = 0;
    float noise = 0; // noise is filtered too, for pink noise.
    int debugCnt = 0;

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
