#pragma once

#include <constants.h>
#include <iostream>

namespace audio::lfo {

enum LFOShape { SIN,
                SQR,
                SAW,
                WAS,
                TRI,
                RND,
                _count };

// dunno if this should be here, rather an evenlope thing really..
enum RampState { OFF,
                 ATTACK,
                 ON,
                 RELEASE };

struct Ramp {
    RampState state = OFF;
    float currVal = 0;
};

enum SNHstate {
    LOADED,
    FIRED
};

class Standard {
  public:
    // a struct holding the ramp and the lfo itself.
    void setShape(LFOShape newShape) {
        lfoShape = newShape;
    }

    void setSpeed(float mhz) {
        angle = mhz * 0.001f * TPH_RACK_RENDER_SIZE * (1.0f / TPH_DSP_SR);
    }

    float getLFOval() {
        float r;
        switch (lfoShape) {
        case LFOShape::SIN:
            // pick val from shared LUT, TBA
            r = AudioMath::csin(phase);
            break;
        case LFOShape::SQR:
            // pick val from shared LUT
            r = (phase > 0.5f) ? 1.0f : -1.0f;
            break;
        case LFOShape::SAW:
            r = phase * 2.0f - 1.0f;
            break;
        case LFOShape::WAS:
            r = (2.0 - phase * 2.0f) - 1.0f;
            break;
        case LFOShape::TRI:
            r = (phase > 0.5f) ? phase : 1.0f - phase;
            r = r * 2.0f - 1.0f;
            break;
        case LFOShape::RND:
            // on, <0.5 => load, on >0.5 => fire
            if (phase < 0.5f && stateSNH == FIRED) {
                stateSNH = LOADED;
            }
            if (phase >= 0.5f && stateSNH == LOADED) {
                lastSNH = AudioMath::noise();
                stateSNH = FIRED;
            }
            r = lastSNH;
            break;
        }
        return r;
    }

    void updatePhase() {
        phase += angle;
        if (phase >= 1)
            phase = phase - 1;
    }

  private:
    double phase = 0;
    double angle = 0;
    SNHstate stateSNH;
    float lastSNH;
    LFOShape lfoShape = LFOShape::SIN;
};
} // namespace audio::lfo