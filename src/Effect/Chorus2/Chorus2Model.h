#pragma once

// #include "./LowPassFilter.h"
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

namespace Effect::Chorus2 {

class Model : public EffectInterface {
  public:
    // Constructor
    Model(float *audioBuffer, std::size_t bufferSize);
    // Public methods. These should match interface right (contract)
    void reset() override;

    // Method to parse MIDI commands
    void parseMidi(char cmd, char param1, char param2) override;
    bool pushMyParam(const std::string &name, float val);

    // Method to render the next block of audio
    bool renderNextBlock() override;

  protected:
    float *buffer; // Pointer to audio buffer
    // LowPassFilter LPF;      // Declare the filter object here
    std::size_t bufferSize; // Size of the audio buffer
    std::vector<double> delayBuffer;
    int delayBufferSize = 32768;
    int delayBufferMask = delayBufferSize - 1;
    int wrIndex = 0;
    int rdIndex = 0; // Changed to rdIndex
    float mix = 0.3f;
    float feedback = 0.3;
    float time = 0.02; // 20 mS, chorus effect..
    int delaySamples = 100;
    float lfoPhase = 0.0f;
    float lfoFrequency = 2.5f;
    float lfoDepth = 0.1; // really has no precice unit. 0.8 round P-P 100 cent.
    //
    float cutoff = 1200;
    float alpha = exp(-2.0 * M_PI * cutoff / TPH_DSP_SR);
    double prevLPF = 0;
    //
    void initializeParameters();
    // Handle incoming MIDI CC messages
    void handleMidiCC(int ccNumber, float value);
    //
    float processLPF(float input);
    void setupParams();
    float modulateLFO(int samples);
    double cubicInterpolate(float delayTimeSamples);
    int debugCnt = 0;
};

} // namespace Effect::Chorus2
