#include <array>
#include <cmath>

class AudioMath {
  public:
    AudioMath() {
        generateLUT();
    }

    // Convert MIDI note to frequency
    float noteToHz(int note, int cent = 0) const {
        return this->masterTune * std::exp2((note - 69 + (cent * 0.01f)) / 12.0f);
    }

    void setMasterTune(float newMasterTune) {
        this->masterTune = newMasterTune;
    }

    float getMasterTune() {
        return this->masterTune;
    }

    // Get sine value from LUT
    inline float sin(float rad) const {
        return lut[static_cast<size_t>(radToIndex(rad)) % lutSize];
    }

    // Get cosine value from LUT
    inline float cos(float rad) const {
        return lut[static_cast<size_t>(radToIndex(rad) + lutSize / 4) % lutSize];
    }

  private:
    // Size of the LUT
    static constexpr size_t lutSize = 1024;
    std::array<float, lutSize> lut;
    float masterTune = 440.0f;

    // Generate the LUT with sine values
    void generateLUT() {
        for (size_t i = 0; i < lutSize; ++i) {
            float rad = (2.0f * M_PI * i) / lutSize;
            lut[i] = std::sin(rad);
        }
    }

    // Convert radians to LUT index
    inline float radToIndex(float rad) const {
        // Normalize rad to the range [0, 2π)
        rad = std::fmod(rad, 2.0f * M_PI);
        if (rad < 0.0f)
            rad += 2.0f * M_PI;
        return (rad / (2.0f * M_PI)) * lutSize;
    }
};
