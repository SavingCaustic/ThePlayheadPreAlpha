#pragma once

#include "./AudioErrorBuffer.h"
#include "./ErrorBuffer.h"
#include "./ErrorRec.h"
#include "AudioErrorProxy.h"
#include "ErrorReader.h"
#include <array>
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

class ErrorHandler {
  public:
    ErrorHandler(AudioErrorBuffer *audioErrorBuffer, ErrorBuffer *errorBuffer)
        : audioErrorBuffer(audioErrorBuffer), errorBuffer(errorBuffer), stop(false) {
        errorProxy = std::make_unique<AudioErrorProxy>(audioErrorBuffer, errorBuffer);
        errorReader = std::make_unique<ErrorReader>(*errorBuffer);
    }

    // Start the threads
    void start() {
        // Start the proxy thread (runs proxyThreadFunc with condition variable)
        proxyThread = std::thread(&AudioErrorProxy::proxyThreadFunc, errorProxy.get());

        // Start the reader thread to process the buffered errors
        readerThread = std::thread(&ErrorHandler::readerLoop, this);
    }

    // Stop the threads safely
    void stopThreads() {
        errorProxy->stop(); // Stop the proxy thread gracefully
        stop = true;

        // Join threads
        std::cout << "trying to stop error-proxy-thread..";
        if (proxyThread.joinable()) {
            proxyThread.join();
        }
        std::cout << "..success!" << std::endl;
        std::cout << "trying to stop error-reader-thread..";
        if (readerThread.joinable()) {
            readerThread.join();
        }
        std::cout << "..success!" << std::endl;
    }

  private:
    void readerLoop() {
        while (!stop) {
            // Process and log errors from the ErrorBuffer
            errorReader->readAll();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Reader can use a small sleep to avoid excessive CPU
        }
    }

    AudioErrorBuffer *audioErrorBuffer;
    ErrorBuffer *errorBuffer;
    std::atomic<bool> stop;
    std::thread proxyThread;
    std::thread readerThread;

    std::unique_ptr<AudioErrorProxy> errorProxy;
    std::unique_ptr<ErrorReader> errorReader;
};
