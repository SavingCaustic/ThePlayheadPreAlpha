#include "AudioMath.h"
#include <iostream>

// Initialize static members
std::array<float, AudioMath::sineLutSize> AudioMath::sineLut{};
float AudioMath::masterTune = 432.0f;
int AudioMath::noiseSeed = 235325325;
int AudioMath::noiseA = 1664525;
int AudioMath::noiseB = 1013904223;
int AudioMath::noiseC = 1 << 24; // 2^24, replaced with constexpr.

// Implement static methods
float AudioMath::noteToHz(int note, int cent) {
    return masterTune * std::exp2((note - 69 + (cent * 0.01f)) * (1 / 12.0f));
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

float AudioMath::catmull(float *lut, int lutSize, float angle) {
    while (angle >= 1)
        angle--;
    float index = angle * lutSize;
    int i = static_cast<int>(index);
    float frac = index - i; // Fractional part for interpolation
    int mask = lutSize - 1;
    float P0 = lut[(i - 1) & mask];
    float P1 = lut[i & mask];
    float P2 = lut[(i + 1) & mask];
    float P3 = lut[(i + 2) & mask];
    return P1 + 0.5f * frac * (P2 - P0 + frac * (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3 + frac * (3.0f * (P1 - P2) + P3 - P0)));
}

float AudioMath::sin(float rad) {
    return catmull(sineLut.data(), sineLutSize, rad * (1 / M_PI / 2));
}

float AudioMath::cos(float rad) {
    return catmull(sineLut.data(), sineLutSize, 0.25f + rad * (1 / M_PI / 2));
}

float AudioMath::csin(float cf) {
    return catmull(sineLut.data(), sineLutSize, cf);
}

float AudioMath::ccos(float cf) {
    return catmull(sineLut.data(), sineLutSize, cf + 0.25f);
}

float AudioMath::ctan(float cf) {
    float sin = AudioMath::csin(cf);
    float cos = fmax(0.001, AudioMath::csin(cf));
    return (sin / cos);
}

void AudioMath::generateLUT() {
    // std::cout << "generating lut" << std::endl;
    for (size_t i = 0; i < sineLutSize; ++i) {
        float rad = (2.0f * M_PI * i) / sineLutSize;
        sineLut[i] = std::sin(rad);
    }
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
