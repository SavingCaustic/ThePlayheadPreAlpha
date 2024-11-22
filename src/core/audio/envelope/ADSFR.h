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
    float factor = 0;    // time-based exp. factor
    float gap = 0;
};

class ADSFR {
  public:
    void setTime(ADSFRState state, float time);
    void setLevel(ADSFRState state, float level);
    void setLeak(ADSFRState state, float level);
    void triggerSlope(Slope &slope, ADSFRCmd cmd);
    void updateDelta(Slope &slope);
    void commit(Slope &slope);

  private:
    float sLevel = 0.5f;
    float aFactor = 0;
    float dFactor = 0;
    float fFactor = 0;
    float rFactor = 0;

    float calcDelta(float time) const;
    void setSlopeState(Slope &slope, ADSFRState state);
    void stateChange(Slope &slope);
};

} // namespace audio::envelope
