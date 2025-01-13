#pragma once

#include <constants.h>
#include <iostream>

namespace audio::envelope {

enum class ASRState { OFF,
                      ATTACK,
                      SUSTAIN,
                      RELEASE };

enum class ASRCmd { NOTE_ON,
                    NOTE_OFF,
                    NOTE_REON };

struct ASRSlope {
    ASRState state = ASRState::OFF;
    float currVal = 0;
    float targetVal = 0; // used for saturated ramping
    float goalVal = 0;   // real goal
    float factor = 0;    // time-based exp. factor
    float gap = 0;
};

class ASR {
  public:
    void setTime(ASRState state, float time);
    void triggerSlope(ASRSlope &slope, ASRCmd cmd);
    void updateDelta(ASRSlope &slope);
    void commit(ASRSlope &slope);

  private:
    float aFactor = 0;
    float rFactor = 0;

    float calcDelta(float time) const;
    void setSlopeState(ASRSlope &slope, ASRState state);
    void stateChange(ASRSlope &slope);
};

} // namespace audio::envelope
