#pragma once
#include "../core/PlayerEngine.h"
#include <cmath>
#include <iostream>
#include <portaudiocpp/PortAudioCpp.hxx>

// 3) To be extended/replaced with Oboe / coreAudio for android etc.
class AudioDriver {
    typedef float SAMPLE;

  public:
    AudioDriver(PlayerEngine *engine)
        : rtPlayerEngine(engine), is_running(false), sample_rate(TPH_AUDIO_SR), stream(nullptr) {
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
        if (is_running)
            return true;

        vol = 0;
	//DONT think this  should be here.
        rtPlayerEngine->doReset(); // Use instance method

        PaError err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, TPH_AUDIO_BUFFER_SIZE, audioCallback, this);
        if (err != paNoError) {
            std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

	Pa_Sleep(100);	//edded to avoid start underruns.
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
    void setSampleRate(double rate) {
        sample_rate = rate;
        if (is_running) {
            stop();
            start();
        }
    }

    // Get sample rate
    double getSampleRate() const {
        return sample_rate;
    }

  private:
    // Static callback function
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo *timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData) {
        AudioDriver *driver = static_cast<AudioDriver *>(userData);
        if (driver && driver->rtPlayerEngine) {
            float *buffer = static_cast<float *>(outputBuffer);
            driver->rtPlayerEngine->renderNextBlock(buffer, framesPerBuffer);
        }
        return paContinue;
    }

    PlayerEngine *rtPlayerEngine; // Pointer to the PlayerEngine instance
    bool is_running;
    double sample_rate;
    PaStream *stream;
    static int gNumNoInputs; // Static member variable
    static double vol;
};
