#pragma once

#include "./ErrorRec.h"
#include "AudioErrorBuffer.h"
#include "ErrorBuffer.h"
#include <atomic>
#include <iostream>
#include <thread>

// This class forwards errors from the real-time safe AudioErrorBuffer to the main ErrorBuffer.
// It's run in its own thread and uses a condition variable to wake up when there's new data.
class AudioErrorProxy {
  public:
    AudioErrorProxy(AudioErrorBuffer *aeBuffer, ErrorBuffer *eBuffer)
        : audioErrorBuffer(aeBuffer), errorBuffer(eBuffer), stopFlag(false) {}

    void proxyThreadFunc() {
        std::unique_lock<std::mutex> lock(cvMutex);
        while (!stopFlag) {
            // Wait for notification from AudioErrorBuffer
            audioErrorBuffer->cv.wait(lock, [this]() { return stopFlag || audioErrorBuffer->hasData(); });

            if (stopFlag) {
                break;
            }

            // Forward the error(s) from the AudioErrorBuffer to ErrorBuffer
            forward();
        }
    }

    // Function to signal the main buffer that a new error is available
    /* removed. The buffers should have this responsibility internally
    void notify() {
        errorBuffer->cv.notify_one(); // Wake up the reader of the main thread
        std::cout << "notify one.. " << std::endl;
    }
    */

    // Stop the proxy thread (for a graceful shutdown)
    void stop() {
        stopFlag = true;
        // cv.notify_all(); // Ensure the thread wakes up and can exit
    }

    void forward() {
        ErrorRec fwdErr;
        // Forward all available errors
        while (audioErrorBuffer->hasData()) { // Use hasData() instead of comparing wrIndex and rdIndex
            if (audioErrorBuffer->read(fwdErr)) {
                errorBuffer->write(fwdErr); // Forward to main error buffer
                std::cout << "forwarding err from audioBuff to mainBuff" << std::endl;
            }
        }
    }

  private:
    AudioErrorBuffer *audioErrorBuffer; // Pointer to the shared audio error buffer
    ErrorBuffer *errorBuffer;           // Pointer to the shared error buffer
    std::mutex cvMutex;                 // Mutex required for CV waiting (minimal use)
    bool stopFlag;                      // Flag to stop the proxy thread
};
