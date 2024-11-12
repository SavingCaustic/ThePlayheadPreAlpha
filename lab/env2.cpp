#include <cmath>
#include <iostream>

// Define constants
constexpr int SR = 48000;
constexpr int RS = 64;

namespace audio::envelope {

enum ADSRState { OFF,
                 ATTACK,
                 DECAY,
                 SUSTAIN,
                 FADE,
                 RELEASE };

enum ADSRCmd { NOTE_ON,
               NOTE_OFF,
               NOTE_REON };

class Slope {
  public:
    ADSRState state = OFF;
    float currVal = 0;
    float targetVal = 0; // used for saturated ramping
    float goalVal = 0;   // real goal
    float factor = 0;    // time based exp. factor
    float gap = 0;
};

class ADSFR {
  public:
    float sLevel = 0.5f;
    float aFactor = 0;
    float dFactor = 0;
    float fFactor = 0;
    float rFactor = 0;

    void setTime(ADSRState state, float time) {
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

    void setLevel(ADSRState state, float level) {
        sLevel = level;
    }

    void triggerSlope(Slope &slope, ADSRCmd cmd) {
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

    bool updateDelta(Slope &slope) {
        // returns true while slope <> OFF
        if (slope.state == ATTACK) {
            if (slope.currVal > slope.goalVal) {
                stateChange(slope);
            }
        } else {
            if (slope.currVal < slope.goalVal) {
                stateChange(slope);
            }
        }
        // Calculate delta
        slope.gap = (slope.targetVal - slope.currVal) * slope.factor;
        // slope.currVal += slope.gap;
        return slope.state != OFF;
        // return gap * slope.factor * (1.0f / RS);
    }

    void commit(Slope &slope) {
        slope.currVal += slope.gap;
    }

  private:
    float calcDelta(float time) const {
        float totalSamples = time * SR * 0.001f;
        return RS / (totalSamples + 40);
        // return (1 - std::exp(-RS / totalSamples));
    }

    void setSlopeState(Slope &slope, ADSRState state) {
        std::cout << "state-changing to " << state << std::endl;
        switch (state) {
        case ATTACK:
            slope.state = ATTACK;
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
            slope.targetVal = slope.currVal * -0.62f;
            slope.factor = fFactor;
            break;
        case RELEASE:
            slope.state = RELEASE;
            slope.goalVal = 0;
            slope.targetVal = slope.currVal * -0.62f;
            slope.factor = rFactor;
            break;
        case OFF:
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

int main() {
    audio::envelope::ADSFR vca;
    vca.setTime(audio::envelope::ATTACK, 2);
    vca.setTime(audio::envelope::DECAY, 8);
    vca.setTime(audio::envelope::FADE, 2000);
    vca.setTime(audio::envelope::RELEASE, 100);
    vca.setLevel(audio::envelope::SUSTAIN, 0.5f);

    audio::envelope::Slope mySlope;

    float vcaEaser, vcaEaserStep;
    int cnt;
    // Trigger "ON"
    vca.triggerSlope(mySlope, audio::envelope::NOTE_ON);
    for (int i = 0; i < 15; i++) {
        vca.updateDelta(mySlope);
        vcaEaser = mySlope.currVal;
        vcaEaserStep = mySlope.gap * (1.0f / RS);
        for (int j = 0; j < RS; j++) {
            // Minor step
            vcaEaser += vcaEaserStep;
        }
        vca.commit(mySlope);
    }

    // Trigger "OFF"
    vca.triggerSlope(mySlope, audio::envelope::NOTE_OFF);
    // with bool return we'll exit on OFF
    cnt = 0;
    while (vca.updateDelta(mySlope)) {
        vcaEaser = mySlope.currVal;
        vcaEaserStep = mySlope.gap * (1.0f / RS);
        for (int j = 0; j < RS; j++) {
            // Minor step
            vcaEaser += vcaEaserStep;
        }
        cnt++;
        vca.commit(mySlope);
    }
    std::cout << "release count: " << cnt << std::endl;

    return 0;
}
