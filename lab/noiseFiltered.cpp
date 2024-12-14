#include <cmath>
#include <iostream>
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>

// simple noise generator to test audio-device and also its low-level latency (when hitting key)

#define FRAMES_PER_BUFFER 64
#define SAMPLE_RATE 48000
#define DEVICE_ID 6
#define TPH_DSP_SR 48000

namespace audio::filter {

enum FilterType {
    LPF, // Low Pass Filter
    BPF, // Band Pass Filter
    HPF  // High Pass Filter
};

class MultiFilter {
  public:
    void setFilterType(FilterType type) {
        if (filterType != type) {
            filterType = type;
        }
    }

    void setCutoff(float frequency) {
        if (frequency > 0.0f && frequency != cutoffHz) {
            cutoffHz = frequency;
        } else {
            std::cerr << "Invalid or unchanged cutoff frequency: " << frequency << std::endl;
        }
    }

    void setResonance(float res) {
        resonance = fmin(1.0f, fmax(0.0f, res));
    }

    void setBandwidthFactor(float factor) {
        if (factor > 0.0f) {
            bandwidthFactor = factor;
        }
    }

    void initFilter() {
        double RC, RC2;
        double dt = 1.0 / TPH_DSP_SR; // Time step as double
        std::cout << "Initializing filter with cutoff frequency: " << cutoffHz << " and resonance: " << resonance << std::endl;

        switch (filterType) {
        case LPF:
            RC = 1.0f / (2.0f * M_PI * cutoffHz);
            alpha = dt / (RC + dt);
            alpha *= 1.0f + resonance; // Adjust alpha by resonance factor for emphasis
            std::cout << "alpha:" << alpha << " RC:" << RC << std::endl;
            break;

        case BPF:
            RC = 1.0f / (2.0f * M_PI * cutoffHz);
            RC2 = 1.0f / (2.0f * M_PI * (cutoffHz * bandwidthFactor));
            alpha = dt / (RC + dt);
            alpha2 = dt / (RC2 + dt);
            alpha *= 1.0f + resonance; // Apply resonance to the low-pass part
            break;

        case HPF:
            RC = 1.0f / (2.0f * M_PI * cutoffHz);
            alpha = RC / (RC + dt);
            alpha *= 1.0f + resonance; // Apply resonance for a sharper high-pass cutoff
            break;
        }
    }

    float applyFilter(float sample) {
        debugCnt++;
        float rsample = 0.5f;
        switch (filterType) {
        case LPF:
            rsample = alpha * sample + (1.0f - alpha) * previousSample;
            previousSample = rsample;
            if ((debugCnt & (32768 - 1)) == 0) {
                std::cout << rsample << std::endl;
            }
            break;

        case BPF:
            highPassedSample = alpha2 * (sample - previousSample) + (1.0f - alpha2) * highPassedSample;
            previousSample = sample;
            lowPassedSample = alpha * highPassedSample + (1.0f - alpha) * lowPassedSample;
            rsample = lowPassedSample;
            break;

        case HPF:
            highPassedSample = alpha * (sample - previousSample + highPassedSample);
            previousSample = sample;
            rsample = highPassedSample;
            break;
        }
        return rsample;
    }

  private:
    FilterType filterType = LPF;
    double alpha = 0.0;
    double alpha2 = 0.0;
    double previousSample = 0.0;
    double highPassedSample = 0.0;
    double lowPassedSample = 0.0;
    double cutoffHz = 2000.0;
    double resonance = 0.5;
    double bandwidthFactor = 1.5;
    int debugCnt = 0;
};

} // namespace audio::filter

// use /lab/portlist to find devices for machine

float volume = 1.0;

int noiseSeed = 235325325;
int noiseA = 1664525;
int noiseB = 1013904223;
int noiseC = 1 << 24; // 2^24, replaced with constexpr.

float noise() {
    // noiseSeed = (noiseA * noiseSeed + noiseB) % noiseC;
    noiseSeed = (noiseA * noiseSeed + noiseB) & (noiseC - 1);
    constexpr float invNoiseC = (1.0f / (1 << 24));
    return noiseSeed * invNoiseC * 2.0f - 1.0f;
}

// Callback function to generate white noise
static int noiseCallback(const void *inputBuffer,
                         void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    float *out = (float *)outputBuffer;
    audio::filter::MultiFilter *filter = (audio::filter::MultiFilter *)userData; // Get filter from userData
    (void)inputBuffer;                                                           // Unused

    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        // Generate random noise for stereo output
        float sample = volume * noise() * 0.25f;
        float filteredSample = filter->applyFilter(sample);
        *out++ = filteredSample;
        *out++ = filteredSample;
    }

    return paContinue; // Continue streaming
}

audio::filter::MultiFilter multiFilter;

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    multiFilter.setFilterType(audio::filter::LPF);
    multiFilter.setCutoff(14000);
    multiFilter.setResonance(0.0);
    multiFilter.initFilter();

    // Get the USB audio device index
    int deviceIndex = DEVICE_ID; // Replace with the actual index if needed

    PaStreamParameters outputParameters;
    outputParameters.device = deviceIndex;
    outputParameters.channelCount = 2;         // Stereo
    outputParameters.sampleFormat = paFloat32; // Use float for output
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    PaStream *stream;
    err = Pa_OpenStream(&stream,
                        NULL,              // Input parameters (NULL for no input)
                        &outputParameters, // Output parameters
                        SAMPLE_RATE,       // Sample rate
                        FRAMES_PER_BUFFER, // Frames per buffer
                        paClipOff,         // Stream flags (use paClipOff to avoid clipping)
                        noiseCallback,     // Callback function
                        &volume);          // User data
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return -1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        Pa_Terminate();
        return -1;
    }

    printf("Playing noise... Press Enter to lower.\n");
    getchar(); // Wait for user input to stop
    volume = 0.3;
    printf("Playing noise... Press Enter to higher.\n");
    getchar(); // Wait for user input to stop
    volume = 0.8;
    printf("Playing noise... Press Enter to stop.\n");
    getchar(); // Wait for user input to stop

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}
