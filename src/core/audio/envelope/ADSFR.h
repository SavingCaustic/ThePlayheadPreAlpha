#pragma once

#include <constants.h>
#include <iostream>

namespace audio::envelope {

enum ADSFRState { OFF,
                  ATTACK,
                  DECAY,
                  SUSTAIN,
                  FADE,
                  RELEASE };

enum ADSFRCmd { NOTE_ON,
                NOTE_OFF,
                NOTE_REON };

struct Slope {
    ADSFRState state = OFF;
    float currVal = 0;
    float targetVal = 0; // used for saturated ramping
    float goalVal = 0;   // real goal
    float factor = 0;    // time based exp. factor
    float gap = 0;
};

class ADSFR {
  public:
    void setTime(ADSFRState state, float time) {
        switch (state) {
        case ATTACK:
            aFactor = calcDelta(time);
            break;
        case DECAY:
            dFactor = calcDelta(time);
            break;
        case FADE:
            fFactor = calcDelta(time);
            break;
        case RELEASE:
            rFactor = calcDelta(time);
            break;
        default:
            break;
        }
    }

    void setLevel(ADSFRState state, float level) {
        sLevel = level;
    }

    void setLeak(ADSFRState state, float level) {
        // experimental - for fade..
        fFactor = level / 1000;
    }

    void triggerSlope(Slope &slope, ADSFRCmd cmd) {
        switch (cmd) {
        case NOTE_ON:
            setSlopeState(slope, ATTACK);
            break;
        case NOTE_OFF:
            setSlopeState(slope, RELEASE);
            break;
        case NOTE_REON:
            // sus-pedal or maybe if noteon=playing note and vel < 64.
            setSlopeState(slope, FADE);
            break;
        }
    }

    void updateDelta(Slope &slope) {
        // returns true while slope <> OFF
        if (slope.state == ATTACK) {
            if (slope.currVal > slope.goalVal) {
                stateChange(slope);
            }
        } else {
            // if sustain-phase, allow for online change of level.
            if (slope.state == FADE) {
                // Not working and incorrect
                // slope.goalVal = sLevel;
                // slope.currVal = sLevel;
            }
            if (slope.currVal < slope.goalVal) {
                stateChange(slope);
            }
        }
        // Calculate delta
        slope.gap = (slope.targetVal - slope.currVal) * slope.factor;
    }

    void commit(Slope &slope) {
        slope.currVal += slope.gap;
    }

  private:
    float sLevel = 0.5f;
    float aFactor = 0;
    float dFactor = 0;
    float fFactor = 0;
    float rFactor = 0;

    float calcDelta(float time) const {
        float totalSamples = time * TPH_DSP_SR * 0.001f;
        return TPH_RACK_RENDER_SIZE / (totalSamples + 40);
    }

    void setSlopeState(Slope &slope, ADSFRState state) {
        // std::cout << "state-changing to " << state << std::endl;
        switch (state) {
        case ATTACK:
            slope.state = ATTACK;
            // slope.currVal = 0.0f; //??
            slope.goalVal = 1.0f;
            slope.targetVal = 1.3f;
            slope.factor = aFactor;
            break;
        case DECAY:
            slope.state = DECAY;
            slope.goalVal = sLevel;
            slope.targetVal = sLevel * 0.62f;
            slope.factor = dFactor;
            break;
        case SUSTAIN:
            // Not an active state for slope
            break;
        case FADE:
            slope.state = FADE;
            slope.goalVal = 0;
            slope.targetVal = slope.currVal * -0.1f;
            slope.factor = fFactor;
            break;
        case RELEASE:
            slope.state = RELEASE;
            slope.goalVal = 0;
            slope.targetVal = slope.currVal * -0.1f;
            slope.factor = rFactor;
            break;
        case OFF:
            std::cout << "reached off" << std::endl;
            slope.state = OFF;
            slope.currVal = 0;
            slope.goalVal = 0;
            slope.targetVal = 0;
            slope.factor = 0;
            break;
        }
    }

    void stateChange(Slope &slope) {
        switch (slope.state) {
        case ATTACK:
            setSlopeState(slope, DECAY);
            break;
        case DECAY:
            setSlopeState(slope, FADE);
            break;
        case FADE:
            setSlopeState(slope, OFF);
            break;
        case RELEASE:
            setSlopeState(slope, OFF);
            break;
        default:
            break;
        }
    }
};
} // namespace audio::envelope