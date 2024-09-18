#pragma once
#include <array>
#include <cmath>

class AudioMath {
  public:
    static float noteToHz(int note, int cent = 0);
    static void setMasterTune(float newMasterTune);
    static float getMasterTune();
    static float noise();
    static float sin(float rad);
    static float cos(float rad);
    static float clamp(float value, float min, float max);
    static float scale(float value, float min, float max);
    static float logScale(float value, float min, float max);

  private:
    static constexpr size_t lutSize = 1024;
    static std::array<float, lutSize> lut;
    static float masterTune;
    static int noiseSeed;
    static int noiseA;
    static int noiseB;
    static int noiseC;

    static void generateLUT();
    static float radToIndex(float rad);
};
