#include "ADSFR.h"
#include "constants.h"

namespace audio::envelope {

void ADSFR::setTime(ADSFRState state, float time) {
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

void ADSFR::setLevel(ADSFRState state, float level) {
    sLevel = level;
}

void ADSFR::setLeak(ADSFRState state, float level) {
    fFactor = level / 1000;
}

void ADSFR::triggerSlope(Slope &slope, ADSFRCmd cmd) {
    switch (cmd) {
    case NOTE_ON:
        setSlopeState(slope, ATTACK);
        break;
    case NOTE_OFF:
        setSlopeState(slope, RELEASE);
        break;
    case NOTE_REON:
        setSlopeState(slope, FADE);
        break;
    }
}

void ADSFR::updateDelta(Slope &slope) {
    if (slope.state == ATTACK) {
        if (slope.currVal > slope.goalVal) {
            stateChange(slope);
        }
    } else {
        if (slope.state == FADE) {
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

void ADSFR::commit(Slope &slope) {
    slope.currVal += slope.gap;
}

float ADSFR::calcDelta(float time) const {
    float totalSamples = time * TPH_DSP_SR * 0.001f;
    return TPH_RACK_RENDER_SIZE / (totalSamples + 40);
}

void ADSFR::setSlopeState(Slope &slope, ADSFRState state) {
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
        break;
    case FADE:
        slope.state = FADE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.1f;
        slope.factor = fFactor;
        break;
    case RELEASE:
        slope.state = RELEASE;
        slope.goalVal = 0;
        slope.targetVal = slope.currVal * -0.01f;
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

void ADSFR::stateChange(Slope &slope) {
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

} // namespace audio::envelope
