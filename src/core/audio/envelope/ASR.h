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
    float k = 0;         // time based exp. factor
    float kGain = 1;     // keyboard tracking support.
};

class ASR {
  public:
    void setTime(ASRState state, float time);
    void triggerSlope(ASRSlope &slope, ASRCmd cmd);
    void updateDelta(ASRSlope &slope);
    void commit(ASRSlope &slope);

  private:
    float aK = 0;
    float rK = 0;

    float calcDelta(float time) const;
    void setSlopeState(ASRSlope &slope, ASRState state);
    void stateChange(ASRSlope &slope);
};

} // namespace audio::envelope
