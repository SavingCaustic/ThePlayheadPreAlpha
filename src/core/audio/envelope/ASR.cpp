#include "ASR.h"
#include "constants.h"

namespace audio::envelope {

void ASR::setTime(ASRState state, float ohm) {
    switch (state) {
    case ASRState::ATTACK:
        aK = calcDelta(ohm);
        break;
    case ASRState::RELEASE:
        rK = calcDelta(ohm);
        break;
    default:
        break;
    }
}

void ASR::triggerSlope(ASRSlope &slope, ASRCmd cmd) {
    switch (cmd) {
    case ASRCmd::NOTE_ON:
        setSlopeState(slope, ASRState::ATTACK);
        break;
    case ASRCmd::NOTE_OFF:
        setSlopeState(slope, ASRState::RELEASE);
        break;
    case ASRCmd::NOTE_REON:
        setSlopeState(slope, ASRState::ATTACK);
        break;
    }
}

void ASR::updateDelta(ASRSlope &slope) {
    // this needs to be updated - never overshoot..
    if (slope.state == ASRState::ATTACK) {
        if (slope.currVal >= slope.goalVal) {
            stateChange(slope);
        }
    } else if (slope.state == ASRState::RELEASE) {
        if (slope.currVal < slope.goalVal) {
            stateChange(slope);
        }
    }
}

float ASR::calcDelta(float ohm) const {
    return 1000.0f / ohm / TPH_DSP_SR;
}

void ASR::setSlopeState(ASRSlope &slope, ASRState state) {
    switch (state) {
    case ASRState::ATTACK:
        slope.state = ASRState::ATTACK;
        slope.goalVal = 1.0f;
        slope.targetVal = 1.3f;
        slope.k = aK;
        break;
    case ASRState::SUSTAIN:
        slope.state = ASRState::SUSTAIN;
        slope.currVal = slope.goalVal; // 1
        slope.targetVal = 1;
        break;
    case ASRState::RELEASE:
        slope.state = ASRState::RELEASE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.01f;
        slope.k = rK;
        break;
    case ASRState::OFF:
        slope.state = ASRState::OFF;
        slope.currVal = 0;
        slope.goalVal = 0;
        slope.targetVal = 0;
        slope.k = 0;
        break;
    }
}

void ASR::stateChange(ASRSlope &slope) {
    switch (slope.state) {
    case ASRState::ATTACK:
        setSlopeState(slope, ASRState::SUSTAIN);
        break;
    case ASRState::RELEASE:
        setSlopeState(slope, ASRState::OFF);
        break;
    default:
        break;
    }
}

} // namespace audio::envelope
