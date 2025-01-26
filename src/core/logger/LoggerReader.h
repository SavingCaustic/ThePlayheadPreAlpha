#pragma once

#include "LoggerQueue.h"

class LoggerReader {
  public:
    // Use a reference to avoid copying
    LoggerReader(LoggerQueue &loggerQueue) : loggerQueue(loggerQueue), stopFlag(false) {}

    void readAll() {
        // Call the correct method to read all errors
        this->loggerQueue.readAllErrors();
    }

    // Stop the proxy thread (for a graceful shutdown)
    void stop() {
        stopFlag = true;
        loggerQueue.cv.notify_all(); // Ensure the thread wakes up and can exit
    }

  private:
    LoggerQueue &loggerQueue; // Reference to the actual ErrorBuffer
    std::mutex cvMutex;       // Mutex required for CV waiting (minimal use)
    bool stopFlag;            // Flag to stop the proxy thread
};
