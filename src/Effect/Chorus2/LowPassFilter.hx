#pragma once

#include "Effect/EffectInterface.h"
#include "constants.h"
#include "core/audio/AudioMath.h"
#include "core/parameters/params.h"
#include <array>
#include <cstdlib> // for rand()
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Effect::Chorus {

class LowPassFilter {
  public:
    LowPassFilter(float cutoff, float sampleRate); // Declare constructor here
    void setCutoff(float newCutoff);
    float process(float input);

  private:
    float alpha;
    float cutoff;
    float sampleRate;
    float previousOutput;
};
