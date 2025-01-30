#include <chrono>
#include <cmath>
#include <iostream>

// Define constants
constexpr int SR = 48000;
constexpr int BLOCK_SIZE = 64;

namespace audio::envelope {

enum ADSFRState { OFF,
                  ATTACK,
                  DECAY,
                  SUSTAIN,
                  FADE,
                  RELEASE };

enum ADSRCmd { NOTE_ON,
               NOTE_OFF,
               NOTE_REON };

class Slope {
    // this is the "capacitor"
  public:
    ADSFRState state = ADSFRState::OFF;
    float currVal = 0;
    float targetVal = 0; // used for saturated ramping
    float goalVal = 0;   // real goal
    float k = 0;         // time based exp. factor
};

class ADSFR {
    // this is the ENVELOPE - for reference
  public:
    float sLevel = 0.5f;
    float aK = 0;
    float dK = 0;
    float fK = 0;
    float rK = 0;

    void setTime(ADSFRState state, float ohm) {
        switch (state) {
        case ADSFRState::ATTACK:
            aK = calcDelta(ohm);
            break;
        case ADSFRState::DECAY:
            dK = calcDelta(ohm);
            break;
        case ADSFRState::FADE:
            fK = calcDelta(ohm);
            break;
        case ADSFRState::RELEASE:
            rK = calcDelta(ohm);
            break;
        default:
            break;
        }
    }

    void setLevel(ADSFRState state, float level) {
        sLevel = level;
    }

    void triggerSlope(Slope &slope, ADSRCmd cmd) {
        switch (cmd) {
        case ADSRCmd::NOTE_ON:
            setSlopeState(slope, ADSFRState::ATTACK);
            break;
        case ADSRCmd::NOTE_OFF:
            setSlopeState(slope, ADSFRState::RELEASE);
            break;
        case ADSRCmd::NOTE_REON:
            // sus-pedal or maybe if noteon=playing note and vel < 64.
            setSlopeState(slope, ADSFRState::FADE);
            break;
        }
    }

    bool updateDelta(Slope &slope) {
        // returns true while slope <> OFF
        if (slope.state == ADSFRState::ATTACK) {
            if (slope.currVal > slope.goalVal) {
                stateChange(slope);
            }
        } else {
            if (slope.currVal < slope.goalVal) {
                stateChange(slope);
            }
        }
        return slope.state != ADSFRState::OFF;
    }

  private:
    float calcDelta(float ohm) const {
        return 1000.0f / ohm / SR;
    }

    void setSlopeState(Slope &slope, ADSFRState state) {
        std::cout << "state-changing to " << state << std::endl;
        switch (state) {
        case ADSFRState::ATTACK:
            slope.state = ADSFRState::ATTACK;
            slope.goalVal = 1.0f;
            slope.targetVal = 1.3f;
            slope.k = aK;
            break;
        case ADSFRState::DECAY:
            slope.state = ADSFRState::DECAY;
            slope.goalVal = sLevel;
            slope.targetVal = sLevel * 0.62f;
            slope.k = dK;
            break;
        case ADSFRState::SUSTAIN:
            // Not an active state for slope
            break;
        case ADSFRState::FADE:
            slope.state = ADSFRState::FADE;
            slope.goalVal = 0;
            slope.targetVal = slope.currVal * -0.62f;
            slope.k = fK;
            break;
        case ADSFRState::RELEASE:
            slope.state = ADSFRState::RELEASE;
            slope.goalVal = 0;
            slope.targetVal = slope.currVal * -0.62f;
            slope.k = rK;
            break;
        case ADSFRState::OFF:
            slope.state = ADSFRState::OFF;
            slope.currVal = 0;
            slope.goalVal = 0;
            slope.targetVal = 0;
            slope.k = 0;
            break;
        }
    }

    void stateChange(Slope &slope) {
        switch (slope.state) {
        case ADSFRState::ATTACK:
            setSlopeState(slope, ADSFRState::DECAY);
            break;
        case ADSFRState::DECAY:
            setSlopeState(slope, ADSFRState::FADE);
            break;
        case ADSFRState::FADE:
            setSlopeState(slope, ADSFRState::OFF);
            break;
        case ADSFRState::RELEASE:
            setSlopeState(slope, ADSFRState::OFF);
            break;
        default:
            break;
        }
    }
};
} // namespace audio::envelope

int main() {
    audio::envelope::ADSFR vca;
    // maybe these are more like setResistance. ok, let's do it.
    vca.setTime(audio::envelope::ATTACK, 2);
    vca.setTime(audio::envelope::DECAY, 5);
    vca.setTime(audio::envelope::FADE, 1000);
    vca.setTime(audio::envelope::RELEASE, 8000);
    vca.setTime(audio::envelope::SUSTAIN, 0.5f);

    audio::envelope::Slope mySlope;

    float vcaEaser, vcaEaserStep;
    int cnt;

    // Start measuring time for envelope calculation
    auto start = std::chrono::high_resolution_clock::now();

    // Trigger "ON"
    vca.triggerSlope(mySlope, audio::envelope::NOTE_ON);

    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            mySlope.currVal = mySlope.currVal * (1.0f - mySlope.k) + mySlope.targetVal * mySlope.k;
        }
        vca.updateDelta(mySlope); // state-machine check
    }
    cnt = 0;
    // Trigger "OFF"
    vca.triggerSlope(mySlope, audio::envelope::NOTE_OFF);
    while (vca.updateDelta(mySlope)) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            mySlope.currVal = mySlope.currVal * (1.0f - mySlope.k) + mySlope.targetVal * mySlope.k;
        }
        cnt++;
    }

    // Stop measuring time
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Output the profiling results
    std::cout << "Release completed in " << duration.count() << " microseconds." << std::endl;
    std::cout << "Release iteration count is: " << cnt << std::endl;

    return 0;
}
