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
        if (!isStereo) {
            length = sampleSize;
        } else {
            length = sampleSize * 0.5;
        }
        data = samplePtr;
    }

    // New method to expose the data pointer
    const float *getDataPointer() const {
        return data;
    }

    void unmountSample() {
        data = nullptr; // Clear the pointer; ownership is external
        length = 0;
        isStereo = false;
    }

    float getSamplesToBuffer(float *leftBuffer, float *rightBuffer, int bufferSize, float currPos, float rate) {
        if (!data) {
            throw std::runtime_error("Sample data is not mounted.");
        }

        int i = 0;

        // Mono Processing
        if (!isStereo) {
            int samplesToFill = std::min(bufferSize, static_cast<int>((length - currPos) / rate));

            for (i = 0; i < samplesToFill; i++) {
                uint32_t sampleIdx = static_cast<uint32_t>(currPos); // Directly cast currPos
                leftBuffer[i] = data[sampleIdx];
                currPos += rate;
            }
        }
        // Stereo Processing
        else {
            int samplesToFill = std::min(bufferSize, static_cast<int>((length - currPos) / rate));

            for (i = 0; i < samplesToFill; i++) {
                uint32_t sampleIdx = static_cast<uint32_t>(currPos) * 2; // x2 must happen *after* cast
                leftBuffer[i] = data[sampleIdx];
                rightBuffer[i] = data[sampleIdx + 1];
                currPos += rate;
                /*if (currPos > 350000) {
                    std::cout << "interesting" << std::endl;
                }*/
            }
        }

        // Zero-padding
        while (i < bufferSize) {
            leftBuffer[i] = 0.0f;
            if (isStereo)
                rightBuffer[i] = 0.0f;
            i++;
        }

        // Looping behavior
        if (currPos >= length) {
            currPos = 0.0f;
        }

        return currPos;
    }

  public:
    bool isStereo;   // Whether the sample is stereo
    uint32_t length; // Length of the sample in frames (not bytes)
  private:
    const float *data; // Pointer to the sample data (*as interleaved* if stereo)
};

} // namespace audio::sample
