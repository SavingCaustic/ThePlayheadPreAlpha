#pragma once

#include "LoggerBuffer.h"

class LoggerReader {
  public:
    // Use a reference to avoid copying
    LoggerReader(LoggerBuffer &loggerBuffer) : loggerBuffer(loggerBuffer), stopFlag(false) {}

    void readAll() {
        // Call the correct method to read all errors
        this->loggerBuffer.readAllErrors();
    }

    // Stop the proxy thread (for a graceful shutdown)
    void stop() {
        stopFlag = true;
        loggerBuffer.cv.notify_all(); // Ensure the thread wakes up and can exit
    }

  private:
    LoggerBuffer &loggerBuffer; // Reference to the actual ErrorBuffer
    std::mutex cvMutex;         // Mutex required for CV waiting (minimal use)
    bool stopFlag;              // Flag to stop the proxy thread
};
