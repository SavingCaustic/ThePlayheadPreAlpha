#include "ADSFR.h"
#include "constants.h"
#include "core/hallways/AudioHallway.h"
#include <cmath>
#include <iostream>

namespace audio::envelope {

void ADSFR::setTime(ADSFRState state, float ohm) {
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

void ADSFR::setLevel(ADSFRState state, float level) {
    sLevel = level;
}

void ADSFR::setLeak(ADSFRState state, float level) {
    fK = level * 0.00002f;
}

void ADSFR::triggerSlope(ADSFRSlope &slope, ADSFRCmd cmd) {
    switch (cmd) {
    case ADSFRCmd::NOTE_ON:
        setSlopeState(slope, ADSFRState::ATTACK);
        break;
    case ADSFRCmd::NOTE_OFF:
        setSlopeState(slope, ADSFRState::RELEASE);
        break;
    case ADSFRCmd::NOTE_REON:
        // sus-pedal or maybe if noteon=playing note and vel < 64.
        setSlopeState(slope, ADSFRState::FADE);
        break;
    }
}

bool ADSFR::updateDelta(ADSFRSlope &slope) {
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

float ADSFR::calcDelta(float ohm) const {
    return 1000.0f / ohm / TPH_DSP_SR;
}

void ADSFR::setSlopeState(ADSFRSlope &slope, ADSFRState state) {
    switch (state) {
    case ADSFRState::ATTACK: {
        slope.state = ADSFRState::ATTACK;
        slope.goalVal = 1.0f;
        slope.targetVal = 1.3f;
        // clamp (to much overshoot)..
        slope.k = std::fmin(0.09f, aK * slope.kGain);
        LoggerRec logTemp;
        FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "setting attack k to %f", slope.k);
        audioHallway.logMessage(logTemp);
        FORMAT_LOG_MESSAGE(logTemp, LOG_INFO, "..kGain is %f", slope.kGain);
        audioHallway.logMessage(logTemp);
        break;
    }
    case ADSFRState::DECAY:
        slope.state = ADSFRState::DECAY;
        slope.goalVal = sLevel;
        slope.targetVal = sLevel * 0.62f;
        slope.k = dK * slope.kGain;
        break;
    case ADSFRState::SUSTAIN:
        // Not an active state for slope
        break;
    case ADSFRState::FADE:
        slope.state = ADSFRState::FADE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.04f;
        slope.k = fK * slope.kGain;
        break;
    case ADSFRState::RELEASE:
        slope.state = ADSFRState::RELEASE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.12f;
        slope.k = rK * slope.kGain;
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

void ADSFR::stateChange(ADSFRSlope &slope) {
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

} // namespace audio::envelope
