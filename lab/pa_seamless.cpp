#include <cmath>
#include <iostream>
#include <portaudio.h>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 64 // CPU usage sky-rockets on 128 or 64 and audio-in. !?
#define AMPLITUDE 0.5f        // Weak sine wave amplitude
#define DEVICE_ID 5           // Weak sine wave amplitude
// we could add parameter to select device here - currently only default..

// Global variables
bool audioInActive = false;
float sinePhase = 0.0f;
PaStream *stream;

// Sine wave callback function (when audio input is not active)
void generateSineWave(float *output, unsigned long framesPerBuffer) {
    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        float sineValue = AMPLITUDE * sinf(2.0f * M_PI * 440.0f * sinePhase / SAMPLE_RATE); // 440 Hz sine wave
        output[2 * i] = sineValue;                                                          // Left channel
        output[2 * i + 1] = sineValue;                                                      // Right channel
        sinePhase += 1.0f;
        if (sinePhase >= SAMPLE_RATE)
            sinePhase -= SAMPLE_RATE;
    }
}

// Callback function for PortAudio
static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags, void *userData) {
    float *output = (float *)outputBuffer;
    const float *input = (const float *)inputBuffer;

    if (audioInActive && inputBuffer != NULL) {
        // Route audio input to output
        for (unsigned long i = 0; i < framesPerBuffer; i++) {
            output[2 * i] = input[i];     // Left channel (mono input to stereo output)
            output[2 * i + 1] = input[i]; // Right channel (mono input to stereo output)
        }
    } else {
        // Generate sine wave when input is inactive
        generateSineWave(output, framesPerBuffer);
    }

    return paContinue;
}

// Function to start audio stream
void startAudioStream() {
    PaStreamParameters outputParameters, inputParameters;

    // Output (speaker) parameters
    outputParameters.device = DEVICE_ID; //Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = 2; // Stereo output
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    // Input (microphone) parameters
    inputParameters.device = DEVICE_ID; //Pa_GetDefaultInputDevice();
    inputParameters.channelCount = 1; // Mono input
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    // Open the stream with both input and output
    Pa_OpenStream(
        &stream,
        &inputParameters,  // Input parameters (NULL if no input is needed)
        &outputParameters, // Output parameters
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,     // No clipping
        audioCallback, // Callback function
        NULL           // No user data
    );

    Pa_StartStream(stream);
}

int main() {
    // Initialize PortAudio
    Pa_Initialize();

    // Start the audio stream
    startAudioStream();

    char input;
    std::cout << "Warning! Be kind to your ears - check sound-level before proceeeding. Press enter to continue" << std::endl;
    //std::cin >> input;
    while (true) {
        std::cout << "Press 't' to toggle audio input. Press 'q' to quit." << std::endl;
        std::cin >> input;
        if (input == 't') {
            audioInActive = !audioInActive;
            std::cout << (audioInActive ? "Audio input activated - snap your fingers!" : "Audio input deactivated!") << std::endl;
        } else if (input == 'q') {
            break;
        }
    }

    // Cleanup
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "Exiting program." << std::endl;
    return 0;
}
