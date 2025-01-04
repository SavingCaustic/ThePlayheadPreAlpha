#include "portaudio.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

/*
 * Simulate a guitar distortion pedal.
 * Record mono input and output clean and processed stereo output.
 *
 * Note that many of the older ISA sound cards on PCs do NOT support
 * full duplex audio (simultaneous record and playback).
 * And some only support full duplex at lower sample rates.
 */
#define SAMPLE_RATE (48000)
#define PA_SAMPLE_TYPE paFloat32
#define FRAMES_PER_BUFFER (64)
#define DEVICE_ID (6)
#define A_SPEED 1.1f
#define D_SPEED .9999f
#define MIX 1
typedef float SAMPLE;

float CubicAmplifier(float input);
static int fuzzCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData);

/* Non-linear amplifier with soft distortion curve. */
float CubicAmplifier(float input) {
    float output, temp;
    float dist;
    float absInput;
    float distFactor;
    //max dist around +/- 0.5. Zero at 0/1.
    distFactor = (input >= 0) ? 26 : 38;
    distFactor *= (1 - input * input) * (abs(input) + 0.1);
    output = input * distFactor;
    return output;
}

static int gNumNoInputs = 0;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int fuzzCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData) {

    SAMPLE *out = (SAMPLE *)outputBuffer;
    const SAMPLE *in = (const SAMPLE *)inputBuffer;
    unsigned int i;
    float mix = MIX;
    (void)timeInfo; /* Prevent unused variable warnings. */
    (void)statusFlags;
    (void)userData;

    if (inputBuffer == NULL) {
        for (i = 0; i < framesPerBuffer; i++) {
            *out++ = 0; /* left - silent */
            *out++ = 0; /* right - silent */
        }
        gNumNoInputs += 1;
    } else {
        for (i = 0; i < framesPerBuffer; i++) {
            SAMPLE sample = *in++; /* MONO input */
            SAMPLE mid = CubicAmplifier(sample);     // * gain;
            *out++ = (mix * mid + (1 - mix) * sample); /* left - distorted */
            *out++ = (mix * mid + (1 - mix) * sample); /* right - distorted */
        }
    }

    return paContinue;
}

/*******************************************************************/
int main(void);
int main(void) {
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream;
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError)
        goto error;

    inputParameters.device = DEVICE_ID; // Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        goto error;
    }
    inputParameters.channelCount = 1; /* mono input */
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = DEVICE_ID; // Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        goto error;
    }
    outputParameters.channelCount = 2; /* stereo output */
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        0, /* paClipOff, */ /* we won't output out of range samples so don't bother clipping them */
        fuzzCallback,
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

    printf("Finished. gNumNoInputs = %d\n", gNumNoInputs);
    Pa_Terminate();
    return 0;

error:
    Pa_Terminate();
    fprintf(stderr, "An error occurred while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return -1;
}
