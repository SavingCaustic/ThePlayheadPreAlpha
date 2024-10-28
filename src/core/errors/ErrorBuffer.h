#pragma once

#include "./ErrorRec.h"
#include <array>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

class ErrorBuffer {
  public:
    static constexpr size_t BufferSize = 64; // Fixed size of the buffer
    int bufferSizeMask = BufferSize - 1;
    std::condition_variable cv; // Condition variable for consumers to wait

    ErrorBuffer() : wrIndex(0), rdIndex(0), full(false) {}

    // Thread-safe write with mutex (only for writer threads)
    void write(const ErrorRec &error) {
        std::lock_guard<std::mutex> lock(mtx); // Lock the mutex for thread-safe access

        // Write to the buffer at the current wrIndex
        buffer[wrIndex] = error;

        // Move the write index forward
        wrIndex = (wrIndex + 1) & bufferSizeMask;

        // If buffer is full, move the read index forward to overwrite old data
        if (full) {
            rdIndex = (rdIndex + 1) & bufferSizeMask;
        }

        // Buffer is full when the write index catches up to the read index
        full = (wrIndex == rdIndex);

        // Notify waiting threads that new data is available
        cv.notify_one();
    }

    // Non-thread-safe read all elements (read by one thread only)
    void readAllErrors() {
        ErrorRec err;
        full = false;
        while (rdIndex != wrIndex || full) {
            err = buffer[rdIndex];
            rdIndex = (rdIndex + 1) & bufferSizeMask;
            // std::cout << "Error: " << err.code << " / " << err.message << std::endl;
            full = false; // Once we read, the buffer can't be full
        }
    }

    // Thread-safe read with condition variable (for consumers)
    void readNextError(ErrorRec &err) {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait until there's data to read
        cv.wait(lock, [this]() { return (rdIndex != wrIndex || full); });

        // Read from the buffer
        if (rdIndex != wrIndex) {
            err = buffer[rdIndex];
            rdIndex = (rdIndex + 1) & bufferSizeMask;
            full = false; // After reading, the buffer cannot be full
        }
    }

  private:
    ErrorRec buffer[BufferSize]; // Ring buffer storage
    size_t wrIndex;              // Write index
    size_t rdIndex;              // Read index
    bool full;                   // Indicates if the buffer is full
    std::mutex mtx;              // Mutex for thread-safe writing
};
