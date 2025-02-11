#include <alsa/asoundlib.h>
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>

// Custom error handler to suppress ALSA messages
void alsaErrorHandler(const char *file, int line, const char *function, int err, const char *fmt, ...) {
    // Do nothing to suppress the messages
    // Uncomment the following line to log suppressed messages (for debugging):
    // fprintf(stderr, "ALSA suppressed: %s:%d %s\n", file, line, function);
}

void suppressAlsaLogs() {
    snd_lib_error_set_handler(alsaErrorHandler);
}

// simple noise generator to test audio-device and also its low-level latency (when hitting key)

#define FRAMES_PER_BUFFER 64
#define SAMPLE_RATE 48000
#define DEVICE_ID 6

// use /lab/portlist to find devices for machine

float volume = 1.0;

int noiseSeed = 235325325;
int noiseA = 1664525;
int noiseB = 1013904223;
int noiseC = 1 << 24; // 2^24, replaced with constexpr.
float noiseEMA = 0.0f;
float gain = 0.0f;
bool gainup = true;

float noise() {
    // noiseSeed = (noiseA * noiseSeed + noiseB) % noiseC;
    noiseSeed = (noiseA * noiseSeed + noiseB) & (noiseC - 1);
    constexpr float invNoiseC = (1.0f / (1 << 24));
    float sample = noiseSeed * invNoiseC * 2.0f - 1.0f;
    // testing use of EMA as EQ..
    noiseEMA = noiseEMA * 0.95f + sample * 0.05f;

    return noiseEMA * 0.5f;
}

// Callback function to generate white noise
static int noiseCallback(const void *inputBuffer,
                         void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    float *out = (float *)outputBuffer;
    (void)inputBuffer; // Unused

    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        // Generate random noise for stereo output
        *out++ = volume * noise() * 0.5f;
        *out++ = volume * noise() * 0.5f;
    }

    return paContinue; // Continue streaming
}

int main() {
    // Set environment variables
    setenv("PA_ALSA_PLUGHW", "1", 1);
    setenv("ALSA_DEBUG", "0", 1);
    setenv("JACK_NO_START_SERVER", "1", 1);
    //
    printf("Before..\n");
    suppressAlsaLogs();
    PaError err = Pa_Initialize();
    printf("After..\n");
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

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
