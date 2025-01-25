#pragma once

#include "./LoggerRec.h"
#include "AudioLoggerBuffer.h"
#include "LoggerBuffer.h"
#include <atomic>
#include <iostream>
#include <thread>

// This class forwards errors from the real-time safe AudioErrorBuffer to the main ErrorBuffer.
// It's run in its own thread and uses a condition variable to wake up when there's new data.
class AudioLoggerProxy {
  public:
    AudioLoggerProxy(AudioLoggerBuffer *alogBuffer, LoggerBuffer *logBuffer)
        : alogBuffer(alogBuffer), logBuffer(logBuffer), stopFlag(false) {}

    void proxyThreadFunc() {
        std::unique_lock<std::mutex> lock(cvMutex);
        while (!stopFlag) {
            // Wait for notification from AudioErrorBuffer
            alogBuffer->cv.wait(lock, [this]() { return stopFlag || alogBuffer->hasData(); });

            if (stopFlag) {
                break;
            }

            // Forward the error(s) from the AudioErrorBuffer to ErrorBuffer
            forward();
        }
    }

    // Stop the proxy thread (for a graceful shutdown)
    void stop() {
        stopFlag = true;
        alogBuffer->cv.notify_all(); // Ensure the thread wakes up and can exit
    }

    void forward() {
        LoggerRec fwdErr;
        // Forward all available errors
        while (alogBuffer->hasData()) { // Use hasData() instead of comparing wrIndex and rdIndex
            if (alogBuffer->read(fwdErr)) {
                logBuffer->write(fwdErr); // Forward to main error buffer
                // std::cout << "forwarding err from audioBuff to mainBuff" << std::endl;
            }
        }
    }

  private:
    AudioLoggerBuffer *alogBuffer; // Pointer to the shared audio error buffer
    LoggerBuffer *logBuffer;       // Pointer to the shared error buffer
    std::mutex cvMutex;            // Mutex required for CV waiting (minimal use)
    bool stopFlag;                 // Flag to stop the proxy thread
};
