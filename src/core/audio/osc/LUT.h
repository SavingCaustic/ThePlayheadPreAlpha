#pragma once

#include <cmath> // Use <cmath> for C++ compatibility with math functions
#include <constants.h>
#include <iostream>

namespace audio::osc {

class LUT {
  public:
    LUT() {
        // Initialize the waveform array to zero
        for (int i = 0; i < LUT_SIZE; i++) {
            wave[i] = 0.0f;
        }
        wave[0] = 0.0f;
    }

    void applySine(float multiple, float amplitude) {
        for (size_t i = 0; i < LUT_SIZE; ++i) {
            wave[i] += std::sin(2.0f * M_PI * i * multiple / LUT_SIZE) * amplitude;
        }
    }

    void normalize() {
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

    // Provide public access to `wave` array
    // const float *getWave() const { return wave; }

    float getSample(size_t index) const {
        return wave[index];
    }

  private:
    float wave[LUT_SIZE]{};
};

class LUTosc {
  public:
    LUTosc(const LUT &theLut) : lutWave(theLut), lutIdx(0.0f) {}

    inline float getNextSample(float offset) {
        int intOffset = (offset * LUT_SIZE);
        int intIdx = (intOffset + static_cast<int>(lutIdx)) & (LUT_SIZE - 1); // Safeguard with modulus operator
        float y = lutWave.getSample(intIdx) * 0.5f;

        // Update lutIdx and wrap around
        lutIdx += angle;
        if (lutIdx >= LUT_SIZE)
            lutIdx -= LUT_SIZE;

        return y;
    }

    void reset() {
        lutIdx = 0;
    }

    void setAngle(float hz) {
        angle = hz * LUT_SIZE / TPH_DSP_SR;
    }

  private:
    const LUT &lutWave; // Reference to an existing LUT instance
    float lutIdx;       // Current index in the LUT
    float angle;
};

} // namespace audio::osc
