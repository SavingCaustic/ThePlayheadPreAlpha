// #include "src/core/audio/AudioMath.h"
#include "LFO.h"
#include <constants.h>
#include <iostream>

namespace audio::lfo {

enum RampState { OFF,
                 ATTACK,
                 ON,
                 RELEASE };

struct Ramp {
    RampState state = OFF;
    float currVal = 0;
};

class RampLfo {
  public:
    // a struct holding the ramp and the lfo itself.
    void setSpeed(float mhz) {
        angle = mhz * 0.001 * TPH_RACK_RENDER_SIZE / (1 / TPH_DSP_SR);
    }

    float getLFOval() {
        float r;
        lfoShape = TRI;
        switch (lfoShape) {
        case SIN:
            // pick val from shared LUT, TBA
            r = 0.5;
            break;
        case SQR:
            // pick val from shared LUT
            r = (phase > 0.5f) ? 1.0f : -1.0f;
            break;
        case SAW:
            r = phase * 2.0f - 1.0f;
            break;
        case TRI:
            r = (phase > 0.5f) ? phase : 1.0f - phase;
            r = r * 2.0f - 1.0f;
            break;
        case RND: // really SNH so hold until?
            // hmm.. we can't pick a noise value for each request. will become funny..
            // r = AudioMath::noise();
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
    float phase = 0;
    float angle = 0;
    LFOShape lfoShape;
    // we could gain from the common big sine-lut, but not luts for every wave..
};

} // namespace audio::lfo