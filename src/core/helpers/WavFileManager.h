#pragma once
#include <atomic>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Simple WAV file structure that holds pre-loaded buffers

// this is just a stub. Don't know where to place this file really..

struct WavFile {
    std::vector<float> preReadBuffer1;   // Double-buffering: buffer 1 (float audio data)
    std::vector<float> preReadBuffer2;   // Double-buffering: buffer 2 (float audio data)
    std::atomic<bool> usingBuffer1;      // Tracks which buffer is currently being used
    std::atomic<bool> isLoaded;          // Tracks whether the file has finished loading
    std::atomic<size_t> currentPosition; // Current read position in the buffer

    // Additional audio metadata
    unsigned int sampleRate; // Sample frequency (e.g., 44100, 48000)
    bool isStereo;           // True if stereo, false if mono

    WavFile(const std::string &filePath) : usingBuffer1(true), isLoaded(false), currentPosition(0) {
        loadHeader(filePath);
    }

    void loadHeader(const std::string &filePath) {
        // Load WAV header and metadata (such as sample rate, channels)
        // For simplicity, we pretend to read the WAV header here
        // In practice, you'd read this from the WAV file itself
        sampleRate = 44100; // Example value
        isStereo = true;    // Example: assume it's stereo
        std::cout << "WAV header loaded for: " << filePath
                  << " | Sample Rate: " << sampleRate
                  << " | Stereo: " << (isStereo ? "Yes" : "No") << std::endl;
    }

    void preLoadData(const std::string &filePath) {
        // Example of reading from a WAV file and loading into pre-read buffers.
        // We'll assume we load the entire file or part of it into the float buffers.
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile) {
            std::cerr << "Failed to open WAV file: " << filePath << std::endl;
            return;
        }

        // Dummy reading logic - replace this with actual WAV file reading logic
        const size_t bufferSize = 4096; // Example buffer size
        preReadBuffer1.resize(bufferSize);
        preReadBuffer2.resize(bufferSize);

        // Fill the preReadBuffer1 initially with some dummy float data
        for (size_t i = 0; i < bufferSize; ++i) {
            preReadBuffer1[i] = static_cast<float>(i) / bufferSize; // Just example data
            preReadBuffer2[i] = 0.0f;                               // Fill the second buffer later
        }

        isLoaded = true; // Mark the WAV file as loaded
        std::cout << "WAV data pre-loaded into buffers for: " << filePath << std::endl;
    }

    // This function switches between buffers without causing glitches
    float *getCurrentBuffer() {
        return usingBuffer1 ? preReadBuffer1.data() : preReadBuffer2.data();
    }

    void swapBuffers() {
        usingBuffer1 = !usingBuffer1;
    }
};

// The WavFileManager handles multiple WavFile objects
class WavFileManager {
  public:
    // Holds multiple WAV files
    std::vector<WavFile> wavFiles;
    std::thread loadingThread;   // Background thread to pre-load data
    std::atomic<bool> isLoading; // Flag to stop the background loader when necessary

    WavFileManager() : isLoading(false) {}

    // Loads a WAV file into memory
    void loadWavFile(const std::string &filePath) {
        WavFile newWavFile(filePath);
        wavFiles.push_back(newWavFile);
    }

    // Starts preloading in a background thread
    void startPreLoading() {
        isLoading = true;
        loadingThread = std::thread(&WavFileManager::preloadFiles, this);
    }

    // Background function for preloading
    void preloadFiles() {
        while (isLoading) {
            for (auto &wavFile : wavFiles) {
                if (!wavFile.isLoaded) {
                    wavFile.preLoadData("path_to_wav"); // Pre-load data into buffers
                    wavFile.isLoaded = true;
                    // Optionally sleep or manage loading more files
                }
            }
        }
    }

    // Stop loading thread
    void stopLoading() {
        isLoading = false;
        if (loadingThread.joinable()) {
            loadingThread.join();
        }
    }
};
