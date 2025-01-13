#include "portaudio.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define SAMPLE_RATE (48000)
#define PA_SAMPLE_TYPE paFloat32
#define FRAMES_PER_BUFFER (64)
#define DEVICE_ID (6)
#define MIX 1
#define NOISE_GATE_THRESHOLD 0.01f  // Threshold for noise gate
#define MIN_ALPHA 0.001f           // Minimum EMA coefficient
#define MAX_ALPHA 0.5f             // Maximum EMA coefficient

typedef float SAMPLE;

static float vuAvg = 0.0f;  // Persistent VU average
static float filteredSignal = 0.0f;  // Persistent EMA filter value

float calculateEMA(float input, float alpha);
static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData);

float calculateEMA(float input, float alpha) {
    filteredSignal = alpha * input + (1.0f - alpha) * filteredSignal;
    return filteredSignal;
}

static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {

    SAMPLE *out = (SAMPLE *)outputBuffer;
    const SAMPLE *in = (const SAMPLE *)inputBuffer;

    unsigned int i;
    float mix = MIX;

    if (inputBuffer == NULL) {
        for (i = 0; i < framesPerBuffer; i++) {
            *out++ = 0;  // Left - silent
            *out++ = 0;  // Right - silent
        }
    } else {
        for (i = 0; i < framesPerBuffer; i++) {
            SAMPLE sample = *in++;  // Mono input
            // Calculate RMS/EMA VU Average
            vuAvg = 0.01f * fabs(sample) + (1.0f - 0.01f) * vuAvg;

            // Adjust alpha based on VU level
            float alpha = MIN_ALPHA + (MAX_ALPHA - MIN_ALPHA) * vuAvg;

            // Apply noise gate
            if (vuAvg < NOISE_GATE_THRESHOLD) {
                sample = 0.0f;  // Suppress signal
            }

            // Apply EMA low-pass filter
            SAMPLE filtered = calculateEMA(sample, alpha);

            // Mix and output
            *out++ = (mix * filtered + (1 - mix) * sample);  // Left
            *out++ = (mix * filtered + (1 - mix) * sample);  // Right
        }
    }

    return paContinue;
}

int main(void) {
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream;
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError)
        goto error;

    inputParameters.device = DEVICE_ID; 
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        goto error;
    }
    inputParameters.channelCount = 1; 
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = DEVICE_ID; 
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        goto error;
    }
    outputParameters.channelCount = 2; 
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        0,
        audioCallback,
        NULL);
    if (err != paNoError)
        goto error;

    err = Pa_StartStream(stream);
    if (err != paNoError)
        goto error;

    printf("Hit ENTER to stop program.\n");
    getchar();
    err = Pa_CloseStream(stream);
    if (err != paNoError)
        goto error;

    Pa_Terminate();
    return 0;

error:
    Pa_Terminate();
    fprintf(stderr, "An error occurred while using the PortAudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return -1;
}

