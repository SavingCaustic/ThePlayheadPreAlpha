#include "LUT.h"

namespace audio::osc {

// LUT class implementation
LUT::LUT() {
    for (int i = 0; i < LUT_SIZE; i++) {
        wave[i] = 0.0f;
    }
    wave[0] = 0.0f;
}

void LUT::applySine(float multiple, float amplitude) {
    for (size_t i = 0; i < LUT_SIZE; ++i) {
        wave[i] += std::sin(2.0f * M_PI * i * multiple / LUT_SIZE) * amplitude;
    }
}

void LUT::normalize() {
    float maxVal = 0.0f;
    for (int i = 0; i < LUT_SIZE; i++) {
        if (std::abs(wave[i]) > maxVal) {
            maxVal = std::abs(wave[i]);
        }
    }
    if (maxVal > 1.0f) {
        float gain = 1.0f / maxVal;
        for (int i = 0; i < LUT_SIZE; i++) {
            wave[i] *= gain;
        }
    }
}

// LUTosc class implementation
LUTosc::LUTosc(const LUT &theLut) : lutWave(theLut), lutIdx(0.0f), angle(0.0f) {}

float LUTosc::getNextSample(float offset) {
    int intOffset = (offset * LUT_SIZE);
    int intIdx = (intOffset + static_cast<int>(lutIdx)) & (LUT_SIZE - 1);
    float y = lutWave.getSample(intIdx);

    lutIdx += angle;
    if (lutIdx >= LUT_SIZE)
        lutIdx -= LUT_SIZE;

    return y;
}

void LUTosc::reset() {
    lutIdx = 0;
}

void LUTosc::setAngle(float hz) {
    angle = hz * LUT_SIZE / TPH_DSP_SR;
}

} // namespace audio::osc
