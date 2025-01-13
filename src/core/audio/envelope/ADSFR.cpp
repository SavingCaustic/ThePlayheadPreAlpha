#include "ADSFR.h"
#include "constants.h"

namespace audio::envelope {

void ADSFR::setTime(ADSFRState state, float time) {
    switch (state) {
    case ADSFRState::ATTACK:
        aFactor = calcDelta(time);
        break;
    case ADSFRState::DECAY:
        dFactor = calcDelta(time);
        break;
    case ADSFRState::FADE:
        fFactor = calcDelta(time);
        break;
    case ADSFRState::RELEASE:
        rFactor = calcDelta(time);
        break;
    default:
        break;
    }
}

void ADSFR::setLevel(ADSFRState state, float level) {
    sLevel = level;
}

void ADSFR::setLeak(ADSFRState state, float level) {
    fFactor = level * 0.001f;
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
        setSlopeState(slope, ADSFRState::FADE);
        break;
    }
}

void ADSFR::updateDelta(ADSFRSlope &slope) {
    if (slope.state == ADSFRState::ATTACK) {
        if (slope.currVal > slope.goalVal) {
            stateChange(slope);
        }
    } else {
        if (slope.state == ADSFRState::FADE) {
            // Not working and incorrect
            // slope.goalVal = sLevel;
            // slope.currVal = sLevel;
        }
        if (slope.currVal < slope.goalVal) {
            stateChange(slope);
        }
    }
    slope.gap = (slope.targetVal - slope.currVal) * slope.factor;
}

void ADSFR::commit(ADSFRSlope &slope) {
    slope.currVal += slope.gap;
}

float ADSFR::calcDelta(float time) const {
    float totalSamples = time * TPH_DSP_SR * 0.001f;
    return TPH_RACK_RENDER_SIZE / (totalSamples + 40); //+40??
}

void ADSFR::setSlopeState(ADSFRSlope &slope, ADSFRState state) {
    switch (state) {
    case ADSFRState::ATTACK:
        slope.state = ADSFRState::ATTACK;
        slope.goalVal = 1.0f;
        slope.targetVal = 1.3f;
        slope.factor = aFactor;
        break;
    case ADSFRState::DECAY:
        slope.state = ADSFRState::DECAY;
        slope.goalVal = sLevel;
        slope.targetVal = sLevel * 0.62f;
        slope.factor = dFactor;
        break;
    case ADSFRState::SUSTAIN:
        break;
    case ADSFRState::FADE:
        slope.state = ADSFRState::FADE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.1f;
        slope.factor = fFactor;
        break;
    case ADSFRState::RELEASE:
        slope.state = ADSFRState::RELEASE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.01f;
        slope.factor = rFactor;
        break;
    case ADSFRState::OFF:
        slope.state = ADSFRState::OFF;
        slope.currVal = 0;
        slope.goalVal = 0;
        slope.targetVal = 0;
        slope.factor = 0;
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
