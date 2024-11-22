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

  private:
    float wave[LUT_SIZE]{};
};

class LUTosc {
  public:
    explicit LUTosc(const LUT &theLut); // Constructor

    float getNextSample(float offset);
    void reset();
    void setAngle(float hz);

  private:
    const LUT &lutWave;
    float lutIdx;
    float angle;
};

} // namespace audio::osc
