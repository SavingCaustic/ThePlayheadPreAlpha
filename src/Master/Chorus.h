#include <cmath> // For sin()
#include <constants.h>
#include <vector>

class MasterChorus {
  public:
    void processChorus(float in[], float out[]) {
        static float lfoPhase[3] = {0.0f, 1.0f, 2.0f}; // Three LFOs for modulation
        constexpr float sampleRate = 48000.0f;
        constexpr float lfoFrequency = 0.5f;    // 0.5 Hz modulation rate
        constexpr size_t maxDelaySamples = 480; // 10ms delay at 48kHz
        static std::vector<float> delayBuffer(maxDelaySamples * 3, 0.0f);
        static size_t writeIndex = 0;

        constexpr float mix = 0.5f;      // 50% dry/wet mix
        constexpr float feedback = 0.2f; // Feedback for extra lushness

        for (size_t i = 0; i < TPH_RACK_RENDER_SIZE; ++i) {
            float wetSignal = 0.0f;

            for (int j = 0; j < 3; ++j) { // Three voices
                // Calculate delay time based on LFO
                float delayTime = (1.0f + std::sin(lfoPhase[j])) * 0.005f; // Between 5–15ms
                size_t delaySamples = static_cast<size_t>(delayTime * sampleRate);

                // Read delayed sample
                size_t readIndex = (writeIndex + delayBuffer.size() - delaySamples) % delayBuffer.size();
                float delayedSample = delayBuffer[readIndex];

                // Update LFO phase
                lfoPhase[j] += 2.0f * M_PI * lfoFrequency / sampleRate;
                if (lfoPhase[j] > 2.0f * M_PI)
                    lfoPhase[j] -= 2.0f * M_PI;

                // Accumulate wet signal
                wetSignal += delayedSample;
            }

            // Write current input sample to delay buffer
            delayBuffer[writeIndex] = in[i] + wetSignal * feedback;

            // Update write index
            writeIndex = (writeIndex + 1) % delayBuffer.size();

            // Mix wet and dry signals
            out[i] = (1.0f - mix) * in[i] + mix * wetSignal / 3.0f; // Normalize by 3 voices
        }
    }
};
