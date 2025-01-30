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

class ADSFRSlope {
    // this is the "capacitor"
  public:
    ADSFRState state = ADSFRState::OFF;
    float currVal = 0;
    float targetVal = 0; // used for saturated ramping
    float goalVal = 0;   // real goal
    float k = 0;         // time based exp. factor
};

class ADSFR {
  public:
    void setTime(ADSFRState state, float time);
    void setLevel(ADSFRState state, float level);
    void setLeak(ADSFRState state, float level);
    void triggerSlope(ADSFRSlope &slope, ADSFRCmd cmd);
    bool updateDelta(ADSFRSlope &slope);

  private:
    float sLevel = 0.5f;
    float aK = 0;
    float dK = 0;
    float fK = 0;
    float rK = 0;

    float calcDelta(float time) const;
    void setSlopeState(ADSFRSlope &slope, ADSFRState state);
    void stateChange(ADSFRSlope &slope);
};

} // namespace audio::envelope
