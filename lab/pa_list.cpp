#include <stdio.h>
#include <portaudio.h>

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
        return -1;
    }

    printf("Available PortAudio devices:\n");
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
        printf("%d: %s\n", i, deviceInfo->name);
    }

    // Choose your device index (replace with actual index after printing list)
    int chosenDeviceIndex = 2;  // Example, replace with actual device index

    // Configure the stream parameters
    PaStreamParameters outputParameters;
    outputParameters.device = chosenDeviceIndex;
    outputParameters.channelCount = 2;  // Stereo output
    outputParameters.sampleFormat = paFloat32;  // 32-bit floating point
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    // Open the audio stream
    PaStream *stream;
    err = Pa_OpenStream(&stream, NULL, &outputParameters, 44100, 256, paClipOff, NULL, NULL);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Start the stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Run the audio processing here...

    // Stop the stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    Pa_Terminate();
    return 0;
}
