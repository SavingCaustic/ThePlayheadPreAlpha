#pragma once

#include "ErrorBuffer.h"

class ErrorReader {
  public:
    // Use a reference to avoid copying
    ErrorReader(ErrorBuffer &errorBuffer) : errorBuffer(errorBuffer), stopFlag(false) {}

    void readAll() {
        // Call the correct method to read all errors
        this->errorBuffer.readAllErrors();
    }

    // Stop the proxy thread (for a graceful shutdown)
    void stop() {
        stopFlag = true;
        errorBuffer.cv.notify_all(); // Ensure the thread wakes up and can exit
    }

  private:
    ErrorBuffer &errorBuffer; // Reference to the actual ErrorBuffer
    std::mutex cvMutex;       // Mutex required for CV waiting (minimal use)
    bool stopFlag;            // Flag to stop the proxy thread
};
