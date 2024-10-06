#pragma once
#include <cmath>
#include <functional>
#include <iostream>
#include <portaudiocpp/PortAudioCpp.hxx>

class AudioDriver {
    typedef float SAMPLE;

  public:
    // Constructor
    AudioDriver()
        : is_running(false), sample_rate(TPH_AUDIO_SR), stream(nullptr), callback(nullptr), gNumInputs(0) {
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
    bool start() {
        if (is_running || callback == nullptr)
            return false; // Ensure callback is registered before starting

        PaError err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, TPH_AUDIO_BUFFER_SIZE, audioCallback, this);
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
    }

    // Set sample rate
    bool setSampleRate(double rate) {
        if (is_running) {
            std::cerr << "Audio-driver error: Can't set sampling rate while running " << std::endl;
            return false;
        } else {
            sample_rate = rate;
        }
    }

    // Get sample rate
    double getSampleRate() const {
        return sample_rate;
    }

    // Register the callback function
    void registerCallback(std::function<void(float *, unsigned long)> cb) {
        callback = cb; // Assign the callback function
    }

    int gNumInputs;

  private:
    // Static callback function
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData) {
        AudioDriver *driver = static_cast<AudioDriver *>(userData);
        if (driver && driver->callback) {
            float *buffer = static_cast<float *>(outputBuffer);
            driver->callback(buffer, framesPerBuffer); // Call the registered callback
        }
        return paContinue;
    }

    bool is_running;
    double sample_rate;
    PaStream *stream;
    std::function<void(float *, unsigned long)> callback; // Function pointer for the callback
};
