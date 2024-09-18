#include "AudioMath.h"

// Initialize static members
std::array<float, AudioMath::lutSize> AudioMath::lut{};
float AudioMath::masterTune = 440.0f;
int AudioMath::noiseSeed = 235325325;
int AudioMath::noiseA = 1664525;
int AudioMath::noiseB = 1013904223;
int AudioMath::noiseC = 1 << 24; // 2^24

// Implement static methods
float AudioMath::noteToHz(int note, int cent) {
    return masterTune * std::exp2((note - 69 + (cent * 0.01f)) / 12.0f);
}

void AudioMath::setMasterTune(float newMasterTune) {
    masterTune = newMasterTune;
}

float AudioMath::getMasterTune() {
    return masterTune;
}

float AudioMath::noise() {
    noiseSeed = (noiseA * noiseSeed + noiseB) % noiseC;
    return noiseSeed / static_cast<float>(noiseC) * 2.0f - 1.0f;
}

float AudioMath::sin(float rad) {
    return lut[static_cast<size_t>(radToIndex(rad)) % lutSize];
}

float AudioMath::cos(float rad) {
    return lut[static_cast<size_t>(radToIndex(rad) + lutSize / 4) % lutSize];
}

void AudioMath::generateLUT() {
    for (size_t i = 0; i < lutSize; ++i) {
        float rad = (2.0f * M_PI * i) / lutSize;
        lut[i] = std::sin(rad);
    }
}

float AudioMath::radToIndex(float rad) {
    rad = std::fmod(rad, 2.0f * M_PI);
    if (rad < 0.0f)
        rad += 2.0f * M_PI;
    return (rad / (2.0f * M_PI)) * lutSize;
}

float AudioMath::clamp(float value, float min, float max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

float AudioMath::scale(float value, float min, float max) {
    return min + (max - min) * value; // Linear scaling
}

float AudioMath::logScale(float value, float min, float max) {
    // Example of logarithmic scaling
    float logMin = std::log(min);
    float logMax = std::log(max);
    return std::exp(logMin + value * (logMax - logMin));
}