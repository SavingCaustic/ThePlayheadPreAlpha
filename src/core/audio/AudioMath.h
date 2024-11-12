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
    static float noteToFloat(int note);
    static void setMasterTune(float newMasterTune);
    static float getMasterTune();
    static float noise();
    static float catmull(float *lut, int lutSize, float angle);
    static float csin(float cf);
    static float ccos(float cf);
    static float ctan(float cf);
    static float sin(float rad);
    static float cos(float rad);
    static float clamp(float value, float min, float max);
    static float linScale(float value, float min, float max);
    static float logScale(float value, float minValue, float octaves);

    static void generateLUT();

    static void normalizeLUT(float *lut, unsigned int lutSize);

  private:
    static constexpr size_t sineLutSize = 256; // Can stay here
    static std::array<float, sineLutSize> sineLut;
    static float masterTune;
    static int noiseSeed;
    static int noiseA;
    static int noiseB;
    static int noiseC;

    static float radToIndex(float rad);
};
