#pragma once

#include <cmath>
#include <constants.h>
#include <iostream>

// maybe this should just be a struct..
// but it's important that voices react to changes of these base settings.
namespace Envelope::ADSFR {

constexpr float OFF_LEVEL = 0.001f;

enum STATE {
    ATTACK,
    DECAY,
    SUSTAIN,
    FADE, // fade not a state but needed when calculating time..
    RELEASE,
    OFF
};

struct ADSFRState {
    float level = 0.0f;
    STATE currentState = ATTACK; // or off? i dunno..
};

class ADSFR {
    // class to set time and levels for:
    // attack, decay, sustain, fade and release
    // this class is used to compare with state (not stored here)
  public:
    // these are never used really..
    float aTime;
    float dTime;
    float fTime;
    float rTime;

    void setVal(STATE state, float timeLevel) {
        float factor;
        if (state != SUSTAIN) {
            // calc factor, and don't allow time 0
            if (timeLevel == 0)
                timeLevel = 1;
            factor = (1.0f - std::exp(-1.0f / timeLevel)) / TPH_RACK_RENDER_SIZE;
        }
        switch (state) {
        case ATTACK:
            aTime = timeLevel;
            aFactor = factor;
            break;
        case DECAY:
            dTime = timeLevel;
            dFactor = factor;
            break;
        case SUSTAIN:
            sLevel = timeLevel;
            break;
        case FADE:
            fTime = timeLevel;
            fFactor = factor;
            break;
        case RELEASE:
            rTime = timeLevel;
            rFactor = factor;
            break;
        }
    }

    float calcDelta(STATE &state, float level) {
        float delta;
        switch (state) {
        case ATTACK:
            if (level < 1.0f) {
                delta = (1 - level) / aTime;
            } else {
                state = DECAY;
                delta = (level - sLevel) / dTime; // initialise decay somehow..
            }
            break;
        case DECAY:
            if (level > sLevel) {
                delta = (level - sLevel) / dTime;
            } else {
                // if leveel really low, go off.
                if (level > 0.001) {
                    state = SUSTAIN;
                    delta = (level - 0) / fTime;
                } else {
                    state = OFF;
                }
            }
            break;
        case SUSTAIN:
            // stay where while note on.
            if (level > OFF_LEVEL) {
                delta = (level - 0) / fTime;
            } else {
                state = OFF;
            }
            break;
        case RELEASE:
            // note off
            if (level > OFF_LEVEL) {
                delta = (level - 0) / rTime;
            } else {
                state = OFF;
            }
            break;
        case OFF:
            // if we get here, voice should be released from playing
            delta = 0;
        }
        return delta;
    }

  private:
    float aFactor;
    float dFactor;
    float sLevel;
    float fFactor;
    float rFactor;
};

} // namespace Envelope::ADSFR