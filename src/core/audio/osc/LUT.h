#pragma once

#include <cmath> // For C++ math functions
#include <constants.h>

namespace audio::osc {

class LUT {
  public:
    LUT(); // Constructor

    void applySine(float multiple, float amplitude);
    void normalize();
    // float getSample(size_t index) const;
    inline float getSample(size_t index) const {
        return wave[index];
    }

    // private:
    // public so factory can change this. Not optimal but for now..
    float wave[LUT_SIZE]{};
};

class LUTosc {
  public:
    explicit LUTosc() // Default constructor
        : lutWave(nullptr), lutIdx(0.0f), angle(0.0f) {}

    // Set LUT after creation
    void setLUT(const LUT &theLut) {
        lutWave = &theLut; // Set LUT reference
    }

    float getNextSample(float offset) {
        if (lutWave) {
            int intOffset = (offset * LUT_SIZE);
            int intIdx = (intOffset + static_cast<int>(lutIdx)) & (LUT_SIZE - 1);
            float y = lutWave->getSample(intIdx);

            lutIdx += angle;
            if (lutIdx >= LUT_SIZE)
                lutIdx -= LUT_SIZE;

            return y;
        }
        return 0;
    }

    void reset() {
        lutIdx = 0.0f;
    }

    void setAngle(float hz) {
        // Set oscillator angle (frequency related logic)
        angle = hz;
    }

  private:
    const LUT *lutWave; // Pointer to LUT (nullptr by default)
    float lutIdx;
    float angle;
};

} // namespace audio::osc
