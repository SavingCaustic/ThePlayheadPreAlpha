#pragma once
#include <array>
#include <atomic>
#include <cstring> // For strcpy
#include <iostream>
#include <optional>

// Fixed-size string lengths for the Message struct
constexpr size_t kTargetSize = 10;
constexpr size_t kParamNameSize = 16;

// A message struct with fixed-size strings
struct MessageIn {
    int rackId;
    char target[kTargetSize];       // Fixed-size char array for target
    char paramName[kParamNameSize]; // Fixed-size char array for paramName
    int paramValue;

    // Default constructor (for default initialization)
    MessageIn() : rackId(0), paramValue(0) {
        std::fill(std::begin(target), std::end(target), '\0');       // Initialize target with empty string
        std::fill(std::begin(paramName), std::end(paramName), '\0'); // Initialize paramName with empty string
    }

    // Constructor to initialize the Message struct with values
    MessageIn(int rackId_, const char *target_, const char *paramName_, int paramValue_)
        : rackId(rackId_), paramValue(paramValue_) {
        strncpy(target, target_, kTargetSize - 1);
        target[kTargetSize - 1] = '\0'; // Ensure null termination
        strncpy(paramName, paramName_, kParamNameSize - 1);
        paramName[kParamNameSize - 1] = '\0'; // Ensure null termination
    }
};

class MessageInBuffer {
  public:
    explicit MessageInBuffer(size_t bufferSize)
        : bufferSize(bufferSize), head(0), tail(0) {
        buffer.fill(MessageIn()); // Initialize buffer with default Messages
    }

    // Push a message (Producer)
    bool push(const MessageIn &message) {
        size_t nextHead = (head + 1) % bufferSize;
        if (nextHead == tail) {
            // Buffer is full
            return false;
        }

        buffer[head] = message; // Store the message directly
        head = nextHead;
        return true;
    }

    // Pop a message (Consumer)
    std::optional<MessageIn> pop() {
        if (tail == head) {
            // Buffer is empty
            return std::nullopt;
        }

        MessageIn message = buffer[tail]; // Load the message directly
        tail = (tail + 1) % bufferSize;
        return message;
    }

  private:
    std::array<MessageIn, 10> buffer; // Fixed-size array for messages
    size_t bufferSize;                // Total buffer size
    std::atomic<size_t> head;         // Head index
    std::atomic<size_t> tail;         // Tail index
};
