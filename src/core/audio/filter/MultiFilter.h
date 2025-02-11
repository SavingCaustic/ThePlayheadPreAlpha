#pragma once
#include "core/audio/AudioMath.h"
namespace audio::filter {

// removed bandstop.
enum class FilterType {
    bypass,
    lowPass,
    highPass,
    bandPass,
    _count
};

enum class FilterPoles {
    p2,
    p4,
    _count
};

class MultiFilter {
  public:
    // uhm - this class may be needed to process single samples - not blocks..

    MultiFilter();

    void reset();

    void setFilterType(FilterType flt);
    void setCutoff(float inCutoff);
    void setResonance(float inRes);
    void setPoles(FilterPoles inPoles);

    void initFilter();

    // mono only
    void processBlock(float *buffer, int numSamples, float noiseLevel = 0);
    float processSample(float sample, FilterPoles poles);

  public:
    FilterType filterType = FilterType::lowPass;
    FilterPoles filterPoles = FilterPoles::p2;
    float filterCutoff = 1500;
    float filterResonance = 0.5f;

  private:
    double d1 = 0;
    double d2 = 0;
    double d3 = 0;
    double d4 = 0;
    double a0, a1, a2, b1, b2;
};
} // namespace audio::filter