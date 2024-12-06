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

class Model : public EffectBase {
  public:
    // Constructor
    Model();
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to parse MIDI commands
    void parseMidi(char cmd, char param1, char param2) override;

    // Method to render the next block of audio
    bool renderNextBlock(bool isSterero) override;

    json getParamDefsAsJSON() override {
        return EffectBase::getParamDefsAsJson();
    }

  protected:
    std::vector<float> delayBuffer;
    int wrPointer = 0;
    int rdPointer = 0;
    float mix = 0.3;
    float feedback = 0.2f;
    float time = 0.57f; // 105 bpm
    int debugCnt = 0;

    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(int ccNumber, float value);
    //
    void setupParams();
};

} // namespace Effect::Delay
