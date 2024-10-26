#pragma once
#include <cmath>
#include <constants.h>
#include <functional>
#include <iostream>
#include <portaudiocpp/PortAudioCpp.hxx>

struct AudioDeviceInfo {
    int id;
    std::string name;
    int pbChannels;  // Playback channels
    int recChannels; // Recording channels
};

class AudioDriver {
    typedef float SAMPLE;

  public:
    // Constructor
    AudioDriver()
        : is_running(false), sample_rate(TPH_AUDIO_SR), stream(nullptr), callback(nullptr), gNumInputs(0), gNumOutputs(2), runningDeviceID(-1) {
        // Initialize PortAudio
        if (!AudioDriver::initialize()) {
            std::cerr << "Failed to initialize PortAudio" << std::endl;
            // Handle error (e.g., throw an exception or log an error)
        }
    }

    ~AudioDriver() {
        stop();
        Pa_Terminate(); // Correct way to terminate PortAudio
    }

    // Static method to initialize PortAudio
    static bool initialize() {
        PaError err = Pa_Initialize();
        if (err != paNoError) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }
        return true;
    }

    // Method to start the audio driver
    bool start(int deviceID = -1, int numInputs = 0, int numOutputs = 2) {
        if (is_running || callback == nullptr) {
            return false; // Ensure callback is registered before starting
        }

        PaError err;
        PaStreamParameters inputParameters, outputParameters;

        if (deviceID != -1) {
            const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(deviceID);
            if (deviceInfo == nullptr) {
                std::cerr << "Invalid device ID" << std::endl;
                return false;
            }

            // Setup input/output parameters if needed
            if (numInputs > 0) {
                inputParameters.device = deviceID;
                inputParameters.channelCount = numInputs;
                inputParameters.sampleFormat = paFloat32;
                inputParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
                inputParameters.hostApiSpecificStreamInfo = nullptr; // Use default host API settings
            }

            if (numOutputs > 0) {
                outputParameters.device = deviceID;
                outputParameters.channelCount = numOutputs;
                outputParameters.sampleFormat = paFloat32;
                outputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
                outputParameters.hostApiSpecificStreamInfo = nullptr; // Use default host API settings
            }

            runningDeviceID = deviceID;
            err = Pa_OpenStream(&stream, numInputs > 0 ? &inputParameters : nullptr,
                                numOutputs > 0 ? &outputParameters : nullptr,
                                sample_rate, TPH_AUDIO_BUFFER_SIZE, paClipOff, audioCallback, this);
        } else {
            // Open default stream if no specific device is provided
            err = Pa_OpenDefaultStream(&stream, numInputs, numOutputs, paFloat32, sample_rate, TPH_AUDIO_BUFFER_SIZE, audioCallback, this);
        }

        if (err != paNoError) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

        err = Pa_StartStream(stream);
        if (err != paNoError) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

        is_running = true;
        gNumInputs = numInputs;
        gNumOutputs = numOutputs;
        return true;
    }

    // Method to stop the audio driver
    void stop() {
        if (!is_running)
            return;

        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
        is_running = false;
        runningDeviceID = -1;
    }

    // Set sample rate
    bool setSampleRate(double rate) {
        if (is_running) {
            std::cerr << "Audio-driver error: Can't set sampling rate while running" << std::endl;
            return false;
        } else {
            sample_rate = rate;
            return true;
        }
    }

    // Get sample rate
    double getSampleRate() const {
        return sample_rate;
    }

    // Register the callback function
    void registerCallback(std::function<void(float *, float *, unsigned long)> cb) {
        callback = cb; // Assign the callback function
    }

    std::vector<AudioDeviceInfo> getAvailableDevices() {
        std::vector<AudioDeviceInfo> availDevices; // Ensure this is declared in your class

        int numDevices = Pa_GetDeviceCount();
        if (numDevices < 0) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(numDevices) << std::endl;
            return availDevices; // Return an empty vector on error
        }

        // Iterate through each available device
        for (int i = 0; i < numDevices; ++i) {
            const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
            if (deviceInfo) {
                AudioDeviceInfo device;
                device.id = i;                                     // Set the device ID
                device.name = deviceInfo->name;                    // Set the device name
                device.pbChannels = deviceInfo->maxOutputChannels; // Set playback channels
                device.recChannels = deviceInfo->maxInputChannels; // Set recording channels
                availDevices.push_back(device);                    // Add the device to the vector
            } else {
                std::cerr << "Could not get info for device ID: " << i << std::endl;
            }
        }

        return availDevices;
    }

    // Get number of input channels
    int getNumInputs() const {
        return gNumInputs;
    }

    // Get number of output channels
    int getNumOutputs() const {
        return gNumOutputs;
    }

    int getDeviceID() {
        return runningDeviceID;
    }

    // Get list of available audio devices
    void listDevices() const {
        int numDevices = Pa_GetDeviceCount();
        if (numDevices < 0) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(numDevices) << std::endl;
            return;
        }

        for (int i = 0; i < numDevices; ++i) {
            const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
            std::cout << "Device ID " << i << ": " << deviceInfo->name << std::endl;
        }
    }

  private:
    // Static callback function
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData) {
        AudioDriver *driver = static_cast<AudioDriver *>(userData);
        if (driver && driver->callback) {
            driver->callback(static_cast<float *>(const_cast<void *>(inputBuffer)),
                             static_cast<float *>(outputBuffer), framesPerBuffer);
        }
        return paContinue;
    }
    int runningDeviceID;
    bool is_running;
    double sample_rate;
    PaStream *stream;
    int gNumInputs;
    int gNumOutputs;
    std::function<void(float *, float *, unsigned long)> callback; // Function pointer for the callback
};
