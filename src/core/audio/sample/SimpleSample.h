#pragma once
#include <cstdint> // For int16_t
#include <stdexcept>

namespace audio::sample {

class SimpleSample {
  public:
    SimpleSample() : isStereo(false), length(0), data(nullptr) {}

    void mountSample(const float *samplePtr, uint32_t sampleSize, bool stereoFlag) {
        if (!samplePtr || sampleSize == 0) {
            throw std::invalid_argument("Invalid sample data or size.");
        }
        isStereo = stereoFlag;
        length = sampleSize;
        data = samplePtr;
    }

    void unmountSample() {
        data = nullptr; // Clear the pointer; ownership is external
        length = 0;
        isStereo = false;
    }

    float getSamplesToBuffer(float *stereoBuffer, int bufferSize, float currPos, float rate) {
        if (!data) {
            throw std::runtime_error("Sample data is not mounted.");
        }

        uint32_t intPos = 0;
        int rightOffset = (isStereo) ? 1 : 0;

        // Calculate the number of samples that can be safely read before padding
        // should be called stereoSamplesToFill
        int stereoSamplesToFill = std::min(bufferSize / 2, static_cast<int>((length / (isStereo + 1) - currPos) / rate));
        int i = 0;

        // Fill the buffer with actual sample data
        // This could be SIMD-optimized by calculating an array of currPos i guess..
        for (i = 0; i < stereoSamplesToFill * 2; i += 2) {
            intPos = static_cast<uint32_t>(currPos);
            if (isStereo)
                intPos *= 2;
            stereoBuffer[i] = data[intPos];
            stereoBuffer[i + 1] = data[intPos + rightOffset];
            currPos += rate;
        }

        // Pad the remaining buffer space with zeros if needed
        for (; i < bufferSize; i += 2) {
            stereoBuffer[i] = 0.0f;
            stereoBuffer[i + 1] = 0.0f;
        }

        // Return the updated position, considering end-of-sample logic
        if (currPos >= length) {
            currPos = 0.0f; // Reset to the start if looping is desired
        }

        return currPos;
    }

  public:
    bool isStereo;   // Whether the sample is stereo
    uint32_t length; // Length of the sample in frames (not bytes)
  private:
    const float *data; // Pointer to the sample data
};

} // namespace audio::sample
