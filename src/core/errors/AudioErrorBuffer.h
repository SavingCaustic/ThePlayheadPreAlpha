#pragma once

#include "./ErrorRec.h"
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <fstream>
#include <iostream>

// Global buffer. Writes made by PlayerEngine & c:o. Read by proxy.

class AudioErrorBuffer {
  public:
    // buffer-size must be 2^x
    static constexpr size_t BufferSize = 64;
    int bufferSizeMask = BufferSize - 1;
    std::atomic<size_t> wrIndex{0}; // Write index
    std::atomic<size_t> rdIndex{0}; // Read index
    ErrorRec buffer[BufferSize];    // Ring buffer storage
    std::condition_variable cv;     // Private condition variable

    // maybe we should have a message here that formats error and creates the ErrorRec
    bool addAudioError(int code, const std::string &message) {
        // i would rather have this function as a "facade, gobably available"
        // also i dunno how to best deal with the varchar and keeping the 100-limit, so fake it for now.
        // yes a facade or some global way to access playerEnigne is neccessary. Synths / Effects must *not*
        // have dependecies to neither Rack or PlayerEnigne
        ErrorRec newError;
        newError.code = code;
        // Use strncpy to safely copy the message into the fixed-size array
        strncpy(newError.message, message.c_str(), sizeof(newError.message) - 1);
        // Ensure null termination in case the message exceeds the array size
        newError.message[sizeof(newError.message) - 1] = '\0';
        // here it would be fancy to automatically use the correct buffer (audio / nonAudio)
        return write(newError);
    }

    // Method to check if there's data in the buffer
    bool hasData() const {
        return wrIndex.load(std::memory_order_acquire) != rdIndex.load(std::memory_order_acquire);
    }

    // Non-blocking write
    bool write(const ErrorRec &errorRec) {
        size_t currentWriteIndex = wrIndex.load(std::memory_order_relaxed);
        size_t nextWriteIndex = (currentWriteIndex + 1) & bufferSizeMask;
        size_t currentReadIndex = rdIndex.load(std::memory_order_acquire);

        // Check if the buffer is full (overflow)
        if (nextWriteIndex == (currentReadIndex & bufferSizeMask)) {
            // Buffer is full, overwrite or drop error depending on policy
            // return false to indicate failure, or continue to overwrite oldest
            return false;
        }

        // Store the error record in the buffer
        buffer[currentWriteIndex & bufferSizeMask] = errorRec;
        // Update the write index
        wrIndex.store(nextWriteIndex, std::memory_order_release);
        cv.notify_one();
        return true;
    }

    // Non-blocking read (used by AudioErrorProxy)
    bool read(ErrorRec &error) {
        size_t currentReadIndex = rdIndex.load(std::memory_order_relaxed);
        size_t currentWriteIndex = wrIndex.load(std::memory_order_acquire);

        // Check if there's anything to read (buffer empty)
        if (currentReadIndex == currentWriteIndex) {
            return false; // Nothing to read
        }

        // Read the error record
        error = buffer[currentReadIndex & bufferSizeMask];

        // Increment the read index atomically
        rdIndex.store((currentReadIndex + 1) & bufferSizeMask, std::memory_order_release);
        return true;
    }
};
