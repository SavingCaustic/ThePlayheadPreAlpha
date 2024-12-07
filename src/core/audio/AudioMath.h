#pragma once
#include <array>
#include <cmath>

// possibly move this code to constants.h
#ifndef M_HALFPI
#define M_HALFPI (M_PI / 2.0)
#endif

class AudioMath {
  public:
    static void initialize(); // setup lut. Could be done with constexpr. but for now..
                              // called by.. PlayerEngine..

    inline static float noteToHz(int note, int cent = 0) {
        return masterTune * std::exp2((note - 69 + (cent * 0.01f)) * (1 / 12.0f));
    }

    // static float fnoteToHz(float note);
    inline static float fnoteToHz(float note) {
        // implemented for portamento support. I guess.. Cent relates to bend + possibly PEG.
        return masterTune * std::exp2((note - 69.0f) * (1 / 12.0f));
    }

    inline static void easeLog1(float in, float &out) {
        out = in * 0.01f + out * 0.99f;
    }

    inline static void easeLog2(float in, float &out) {
        out = in * 0.02f + out * 0.98f;
    }

    inline static void easeLog5(float in, float &out) {
        out = in * 0.05f + out * 0.95f;
    }

    inline static void easeLog10(float in, float &out) {
        out = in * 0.1f + out * 0.9f;
    }

    inline static void easeLog20(float in, float &out) {
        out = in * 0.2f + out * 0.8f;
    }

    inline static void easeLog50(float in, float &out) {
        out = in * 0.5f + out * 0.5f;
    }

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

    static float noteToFloat(int note);

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
