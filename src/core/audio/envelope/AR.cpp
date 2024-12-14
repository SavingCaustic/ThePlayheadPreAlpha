#include "AR.h"
#include "constants.h"

namespace audio::envelope {

void AR::setTime(ARState state, float time) {
    switch (state) {
    case ARState::ATTACK:
        aFactor = calcDelta(time);
        break;
    case ARState::RELEASE:
        rFactor = calcDelta(time);
        break;
    default:
        break;
    }
}

void AR::triggerSlope(ARSlope &slope, ARCmd cmd) {
    switch (cmd) {
    case ARCmd::NOTE_ON:
        setSlopeState(slope, ARState::ATTACK);
        break;
    case ARCmd::NOTE_OFF:
        setSlopeState(slope, ARState::RELEASE);
        break;
    case ARCmd::NOTE_REON:
        // uhm
        setSlopeState(slope, ARState::ATTACK);
        break;
    }
}

void AR::updateDelta(ARSlope &slope) {
    if (slope.state == ARState::ATTACK) {
        if (slope.currVal > slope.goalVal) {
            stateChange(slope);
        }
    } else {
        if (slope.currVal < slope.goalVal) {
            stateChange(slope);
        }
    }
    slope.gap = (slope.targetVal - slope.currVal) * slope.factor;
}

void AR::commit(ARSlope &slope) {
    slope.currVal += slope.gap;
}

float AR::calcDelta(float time) const {
    float totalSamples = time * TPH_DSP_SR * 0.001f;
    return TPH_RACK_RENDER_SIZE / (totalSamples + 40);
}

void AR::setSlopeState(ARSlope &slope, ARState state) {
    switch (state) {
    case ARState::ATTACK:
        slope.state = ARState::ATTACK;
        slope.goalVal = 1.0f;
        slope.targetVal = 1.3f;
        slope.factor = aFactor;
        break;
    case ARState::RELEASE:
        slope.state = ARState::RELEASE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.01f;
        slope.factor = rFactor;
        break;
    case ARState::OFF:
        slope.state = ARState::OFF;
        slope.currVal = 0;
        slope.goalVal = 0;
        slope.targetVal = 0;
        slope.factor = 0;
        break;
    }
}

void AR::stateChange(ARSlope &slope) {
    switch (slope.state) {
    case ARState::ATTACK:
        setSlopeState(slope, ARState::RELEASE);
        break;
    case ARState::RELEASE:
        setSlopeState(slope, ARState::OFF);
        break;
    default:
        break;
    }
}

} // namespace audio::envelope
