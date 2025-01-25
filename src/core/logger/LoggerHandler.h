#pragma once

#include "./AudioLoggerBuffer.h"
#include "./LoggerBuffer.h"
#include "./LoggerRec.h"
#include "AudioLoggerProxy.h"
#include "LoggerReader.h"
#include <array>
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

class LoggerHandler {
  public:
    LoggerHandler(AudioLoggerBuffer *audioErrorBuffer, LoggerBuffer *errorBuffer)
        : audioLoggerBuffer(audioLoggerBuffer), loggerBuffer(loggerBuffer), stop(false) {
        loggerProxy = std::make_unique<AudioLoggerProxy>(audioErrorBuffer, errorBuffer);
        loggerReader = std::make_unique<LoggerReader>(*errorBuffer);
    }

    // Start the threads
    void start() {
        // Start the proxy thread (runs proxyThreadFunc with condition variable)
        proxyThread = std::thread(&AudioLoggerProxy::proxyThreadFunc, loggerProxy.get());

        // Start the reader thread to process the buffered errors
        readerThread = std::thread(&LoggerHandler::readerLoop, this);
    }

    // Stop the threads safely
    void stopThreads() {
        loggerProxy->stop(); // Stop the proxy thread gracefully
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
            loggerReader->readAll();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Reader can use a small sleep to avoid excessive CPU
        }
    }

    AudioLoggerBuffer *audioLoggerBuffer;
    LoggerBuffer *loggerBuffer;
    std::atomic<bool> stop;
    std::thread proxyThread;
    std::thread readerThread;

    std::unique_ptr<AudioLoggerProxy> loggerProxy;
    std::unique_ptr<LoggerReader> loggerReader;
};
