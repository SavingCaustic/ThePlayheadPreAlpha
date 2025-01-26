#pragma once
#include <atomic>
#include <condition_variable>
#include <cstring> // For strncpy
#include <iostream>
#include <mutex>
#include <optional>

constexpr size_t msgOutTargetSize = 10;
constexpr size_t msgOutParamNameSize = 16;
constexpr size_t msgOutParamLabelSize = 10;

// Struct for outgoing messages
struct MessageOut {
    int rackId;
    char target[msgOutTargetSize];
    char paramName[msgOutParamNameSize];
    float paramValue;
    char paramLabel[msgOutParamLabelSize];

    MessageOut() : rackId(0), paramValue(0) {
        std::fill(std::begin(target), std::end(target), '\0');
        std::fill(std::begin(paramName), std::end(paramName), '\0');
        std::fill(std::begin(paramLabel), std::end(paramLabel), '\0');
    }

    MessageOut(int rackId_, const char *target_, const char *paramName_, float paramValue_, const char *paramLabel_)
        : rackId(rackId_), paramValue(paramValue_) {
        strncpy(target, target_, msgOutTargetSize - 1);
        target[msgOutTargetSize - 1] = '\0';
        strncpy(paramName, paramName_, msgOutParamNameSize - 1);
        paramName[msgOutParamNameSize - 1] = '\0';
        strncpy(paramLabel, paramLabel_, msgOutParamLabelSize - 1);
        paramLabel[msgOutParamLabelSize - 1] = '\0';
    }
};

class MessageOutQueue {
  public:
    static constexpr size_t kBufferSize = 32; // Set your desired buffer size
    std::condition_variable cv;               // Reference to shared condition variable
    std::mutex mtx;

    MessageOutQueue()
        : head(0), tail(0), messageAvailable(false) {}

    // Push a message (Producer)
    bool push(const MessageOut &message) {
        size_t nextHead = (head.load(std::memory_order_relaxed) + 1) % kBufferSize;
        if (nextHead == tail.load(std::memory_order_acquire)) {
            // Buffer is full
            return false;
        }

        buffer[head.load(std::memory_order_relaxed)] = message; // Store the message directly
        head.store(nextHead, std::memory_order_release);

        // Notify the consumer thread that a message is available
        messageAvailable.store(true, std::memory_order_release);
        cv.notify_one(); // Notify the reader to wake up
        return true;
    }

    // Non-blocking pop function
    std::optional<MessageOut> pop() {
        if (!messageAvailable.load(std::memory_order_acquire)) {
            return std::nullopt; // No messages available
        }

        // Use a lock-free approach to pop the message
        auto currentTail = tail.load(std::memory_order_relaxed);
        auto currentHead = head.load(std::memory_order_relaxed);

        if (currentTail == currentHead) {
            // No messages available
            messageAvailable.store(false, std::memory_order_release);
            return std::nullopt;
        }

        MessageOut message = buffer[currentTail];
        tail.store((currentTail + 1) % kBufferSize, std::memory_order_release);

        // Check if the buffer is now empty after popping
        if (tail.load(std::memory_order_relaxed) == currentHead) {
            messageAvailable.store(false, std::memory_order_release); // No more messages
        }

        return message;
    }

    bool checkMoreMessages() const {
        return tail.load(std::memory_order_relaxed) != head.load(std::memory_order_relaxed);
    }

  private:
    MessageOut buffer[kBufferSize]; // Fixed-size buffer
    std::atomic<size_t> head;       // Atomic head index
    std::atomic<size_t> tail;       // Atomic tail index

    std::atomic<bool> messageAvailable; // Atomic flag to indicate if a message is available
};
