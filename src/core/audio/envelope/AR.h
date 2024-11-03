#pragma once

#include <cmath>
#include <constants.h>
#include <iostream>

namespace audio::envelope {

struct ARState {
    enum STATE {
        ATTACK,
        SUSTAIN,
        RELEASE,
        OFF
    };

    constexpr static float OFF_LEVEL = 0.001f;

    float level = 0.0f;
    STATE currentState = OFF; // or off? i dunno..
};

class AR {
    // class to set time and levels for:
    // attack, decay, sustain, fade and release
    // this class is used to compare with state (not stored here)
  public:
    // these are never used really..
    float aTime;
    float rTime;

    void setVal(ARState::STATE state, float timeLevel) {
        float factor;
        factor = (1.0f - std::exp(-1.0f / (timeLevel * TPH_DSP_SR * 0.001f)));
        switch (state) {
        case ARState::ATTACK:
            aTime = timeLevel;
            aFactor = factor;
            std::cout << "aFactor set to " << aFactor << std::endl;
            break;
        case ARState::RELEASE:
            rTime = timeLevel;
            rFactor = factor;
            std::cout << "rFactor set to " << rFactor << std::endl;
            break;
        case ARState::SUSTAIN:
            break;
        }
    }

    void process(ARState &arState) {
        switch (arState.currentState) {
        case ARState::ATTACK:
            if (arState.level < 1.0f) {
                arState.level = (1.5 - arState.level) * aFactor; // 1.58 = 0.63/1 =>
            } else {
                arState.currentState = ARState::SUSTAIN;
                arState.level = 1;
            }
            break;
        case ARState::SUSTAIN:
            // recover from overdrive
            if (arState.level > 1.0f) {
                arState.level = (1 - arState.level) * aFactor * 0.1;
            }
            break;
        case ARState::RELEASE:
            // note off
            if (arState.level > ARState::OFF_LEVEL) {
                arState.level = (0 - arState.level) * rFactor;
                // edge case (negative amp), caught at render end.
            } else {
                arState.currentState = ARState::OFF;
                arState.level = 0;
            }
            break;
        case ARState::OFF:
            // if we get here, voice should be released from playing
            arState.level = 0;
        }
    }

  private:
    float aFactor;
    float rFactor;
};

} // namespace audio::envelope