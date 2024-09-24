#pragma once
#include <array>
#include <cmath>

// possibly move this code to constants.h
#ifndef M_HALFPI
#define M_HALFPI (M_PI / 2.0)
#endif

class AudioMath {
  public:
    static float noteToHz(int note, int cent = 0);
    static void setMasterTune(float newMasterTune);
    static float getMasterTune();
    static float noise();
    static float csin(float cf);
    static float ccos(float cf);
    static float ctan(float cf);
    static float sin(float rad);
    static float cos(float rad);
    static float clamp(float value, float min, float max);
    static float linScale(float value, float min, float max);
    static float logScale(float value, float minValue, float octaves);

    static void generateLUT();

  private:
    static constexpr size_t lutSize = 1024; // Can stay here
    static const float lutSizeFloat;        // Declaration only, definition in .cpp
    static std::array<float, lutSize> lut;
    static float masterTune;
    static int noiseSeed;
    static int noiseA;
    static int noiseB;
    static int noiseC;

    static float radToIndex(float rad);
};
