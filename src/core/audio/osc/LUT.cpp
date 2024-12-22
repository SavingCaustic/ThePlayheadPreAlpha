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

} // namespace audio::osc
