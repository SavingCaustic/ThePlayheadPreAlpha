#pragma once

#include <constants.h>
#include <iostream>

namespace audio::envelope {

enum class ARState { OFF,
                     ATTACK,
                     RELEASE };

enum class ARCmd { NOTE_ON,
                   NOTE_OFF,
                   NOTE_REON };

struct ARSlope {
    ARState state = ARState::OFF;
    float currVal = 0;
    float targetVal = 0; // used for saturated ramping
    float goalVal = 0;   // real goal
    float factor = 0;    // time-based exp. factor
    float gap = 0;
};

class AR {
  public:
    void setTime(ARState state, float time);
    void setLevel(ARState state, float level);
    void setLeak(ARState state, float level);
    void triggerSlope(ARSlope &slope, ARCmd cmd);
    void updateDelta(ARSlope &slope);
    void commit(ARSlope &slope);

  private:
    float aFactor = 0;
    float rFactor = 0;

    float calcDelta(float time) const;
    void setSlopeState(ARSlope &slope, ARState state);
    void stateChange(ARSlope &slope);
};

} // namespace audio::envelope
