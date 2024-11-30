#pragma once

#include "AdminBuffer.h"

namespace Admin {

class AdminReader {
  public:
    // Use a reference to avoid copying
    AdminReader(AdminBuffer &adminBuffer) : adminBuffer(adminBuffer), stopFlag(false) {}

    void readAll() {
        // Call the correct method to read all errors
        this->adminBuffer.readAllCommands();
    }

    // Stop the proxy thread (for a graceful shutdown)
    void stop() {
        stopFlag = true;
        adminBuffer.cv.notify_all(); // Ensure the thread wakes up and can exit
    }

  private:
    AdminBuffer &adminBuffer; // Reference to the actual ErrorBuffer
    std::mutex cvMutex;       // Mutex required for CV waiting (minimal use)
    bool stopFlag;            // Flag to stop the proxy thread
};
} // namespace Admin
