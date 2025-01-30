#include "./ChorusModel.h"
#include <cmath>
#include <vector>

namespace Effect::Chorus {
// maybe we should pass reference to playerEngine since we may want to send an error?

// Constructor to accept buffer and size
Model::Model() {
    setupParams();                       // creates the array with attributes and lambdas for parameters - NOT INTERFACE
    delayBuffer.resize(delayBufferSize); // must be 2^x since bitmasking.
    std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
    reset(); // Initialize with default values
}

void Model::reset() {
    // Reset write and read indices with appropriate defaults
    wrIndex = static_cast<int>(time * 48000);      // Reset write index
    rdIndex = 0;                                   // Reset read index
    delaySamples = static_cast<int>(time * 48000); // Set delay samples based on time
}

void Model::setupParams() {
    // Initialize any parameters here
}

// maybe let this be a mono chorus with a shared delay line..

bool Model::renderNextBlock(bool isStereo) {
    double delayOutL, delayOutR;
    float audioInL, audioInR;
    for (std::size_t i = 0; i < bufferSize; i += 2) {
        // Read the input samples
        audioInL = bufferLeft[i];
        audioInR = bufferLeft[i]; // mono in?

        // Get the current LFO value and calculate the delay time
        float lfoValue = modulateLFO(1);
        float delayTimeSamples = delaySamples + lfoValue * lfoDepth * 100;

        // Calculate the read index and apply cubic interpolation to get delayed samples
        int readIndex = (wrIndex - static_cast<int>(delayTimeSamples) + delayBufferSize) % delayBufferSize;
        // i can't use audiomath here cause precision lost on float
        delayOutL = cubicInterpolate(delayTimeSamples);
        delayOutR = delayOutL; // cubicInterpolate(delayTimeSamples);

        // Mix the delayed signal with the original input (wet/dry mix)
        float outputSampleL = (1 - this->mix) * audioInL + this->mix * delayOutL; // Mixing delayed sample into the input
        float outputSampleR = (1 - this->mix) * audioInR + this->mix * delayOutR;

        // Write the output back to the buffer
        bufferLeft[i] = outputSampleL;
        bufferRight[i] = outputSampleR;

        // Write the current input samples to the delay buffer (for feedback)
        delayBuffer[wrIndex] = processLPF(audioInL) * (1 - this->feedback) + delayOutL * this->feedback;

        // Increment the write index and wrap it around the buffer size
        wrIndex = (wrIndex + 1) & delayBufferMask;

        // Increment LFO phase
        lfoPhase += 1.0f;
        if (lfoPhase >= TPH_DSP_SR) {
            lfoPhase -= TPH_DSP_SR; // Wrap LFO phase
        }
    }

    return true; // Stereo signal processed
}

float Model::modulateLFO(int samples) {
    float lfoNow = AudioMath::csin(lfoPhase);
    lfoPhase += lfoFrequency * (1.0f / TPH_DSP_SR) * samples;
    if (lfoPhase > 1)
        lfoPhase -= 1;
    return lfoNow;
}

float Model::processLPF(float input) {
    prevLPF = (1.0f - alpha) * input + alpha * prevLPF;
    return prevLPF;
}

double Model::cubicInterpolate(float delayTimeSamples) {
    float index = wrIndex - delayTimeSamples; // Adjust index by delay time
    int i = static_cast<int>(index);
    float frac = index - i; // Fractional part for interpolation

    int mask = delayBufferSize - 1;
    float P0 = delayBuffer[(i - 1) & mask];
    float P1 = delayBuffer[i & mask];
    float P2 = delayBuffer[(i + 1) & mask];
    float P3 = delayBuffer[(i + 2) & mask];
    return P1 + 0.5f * frac * (P2 - P0 + frac * (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3 + frac * (3.0f * (P1 - P2) + P3 - P0)));
}

} // namespace Effect::Chorus
