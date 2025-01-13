#pragma once

#include <constants.h>
#include <iostream>

namespace audio::envelope {

enum class ADSFRState { OFF,
                        ATTACK,
                        DECAY,
                        SUSTAIN,
                        FADE,
                        RELEASE };

enum class ADSFRCmd { NOTE_ON,
                      NOTE_OFF,
                      NOTE_REON };

struct ADSFRSlope {
    ADSFRState state = ADSFRState::OFF;
    float currVal = 0;
    float targetVal = 0; // used for saturated ramping
    float goalVal = 0;   // real goal
    float factor = 0;    // time-based exp. factor
    float gap = 0;
};

class ADSFR {
  public:
    void setTime(ADSFRState state, float time);
    void setLevel(ADSFRState state, float level);
    void setLeak(ADSFRState state, float level);
    void triggerSlope(ADSFRSlope &slope, ADSFRCmd cmd);
    void updateDelta(ADSFRSlope &slope);
    void commit(ADSFRSlope &slope);

  private:
    float sLevel = 0.5f;
    float aFactor = 0;
    float dFactor = 0;
    float fFactor = 0;
    float rFactor = 0;

    float calcDelta(float time) const;
    void setSlopeState(ADSFRSlope &slope, ADSFRState state);
    void stateChange(ADSFRSlope &slope);
};

} // namespace audio::envelope
