#include "AudioMath.h"
#include <iostream>

// Initialize static members
std::array<float, AudioMath::lutSize> AudioMath::lut{};
const float AudioMath::lutSizeFloat = 1024.0f;
float AudioMath::masterTune = 440.0f;
int AudioMath::noiseSeed = 235325325;
int AudioMath::noiseA = 1664525;
int AudioMath::noiseB = 1013904223;
int AudioMath::noiseC = 1 << 24; // 2^24, replaced with constexpr.

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
    // noiseSeed = (noiseA * noiseSeed + noiseB) % noiseC;
    noiseSeed = (noiseA * noiseSeed + noiseB) & (noiseC - 1);
    constexpr float invNoiseC = (1.0f / (1 << 24));
    return noiseSeed * invNoiseC * 2.0f - 1.0f;
}

float AudioMath::sin(float rad) {
    return lut[static_cast<size_t>(radToIndex(rad)) % lutSize];
}

float AudioMath::cos(float rad) {
    return lut[static_cast<size_t>(radToIndex(rad) + lutSize / 4) % lutSize];
}

float AudioMath::csin(float cf) {
    int pos = static_cast<size_t>(cf * lutSizeFloat) & (lutSize - 1);
    return lut[pos];
}

float AudioMath::ccos(float cf) {
    int pos = static_cast<int>((cf + 0.25) * lutSizeFloat) & (lutSize - 1);
    return lut[pos];
}

float AudioMath::ctan(float cf) {
    float sin = AudioMath::csin(cf);
    float cos = AudioMath::csin(cf);
    if (cos == 0)
        cos = 0.001;
    return (sin / cos);
}

void AudioMath::generateLUT() {
    // std::cout << "generating lut" << std::endl;
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

float AudioMath::linScale(float value, float min, float max) {
    return min + (max - min) * value; // Linear scaling
}

float AudioMath::logScale(float value, float minValue, float octaves) {
    // note that input is integer! 0-127. This should be rewritten to 0-1
    float result = minValue * std::exp2(value * octaves);
    std::cout << "log2 of " << value << " is " << result << std::endl;
    return result;
}