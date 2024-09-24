#pragma once
#include <atomic>
#include <cstring> // For strcpy
#include <iostream>
#include <optional>
#include <vector>

// this isn't really a message responder since it's decoupled. The reading of this class is done by the websocket,
// possibly mocked in http-endpoint one-by-one.

// not sure who's responsible for kicking back the response. Possibly the player engine, knowing which rack that's in focus..

// note that the struct of the message is a bit different, including labeled reply value.

// Fixed-size string lengths for the Message struct
constexpr size_t kTargetSize = 10;
constexpr size_t kParamNameSize = 16;
constexpr size_t kParamLabelSize = 10;

// A message struct with fixed-size strings
struct Response {
    int rackId;
    char target[kTargetSize];       // Fixed-size char array for target
    char paramName[kParamNameSize]; // Fixed-size char array for paramName
    float paramValue;
    char paramLabel[kParamLabelSize];

    // Default constructor (for default initialization)
    Response() : rackId(0), paramValue(0) {
        std::fill(std::begin(target), std::end(target), '\0');       // Initialize target with empty string
        std::fill(std::begin(paramName), std::end(paramName), '\0'); // Initialize paramName with empty string
    }

    // Constructor to initialize the Message struct with values
    Response(int rackId_, const char *target_, const char *paramName_, float paramValue_, const char *paramLabel_)
        : rackId(rackId_), paramValue(paramValue_) {
        strncpy(target, target_, kTargetSize - 1);
        target[kTargetSize - 1] = '\0'; // Ensure null termination
        strncpy(paramName, paramName_, kParamNameSize - 1);
        paramName[kParamNameSize - 1] = '\0'; // Ensure null termination
    }
};

class MessageResponder {
  public:
    explicit MessageResponder(size_t bufferSize)
        : buffer(bufferSize), head(0), tail(0) {}

    // Push a message (Producer)
    bool push(const Response &message) {
        size_t nextHead = (head + 1) % buffer.size();
        if (nextHead == tail) {
            // Buffer is full
            return false;
        }

        buffer[head].store(message, std::memory_order_release);
        head = nextHead;
        return true;
    }

    // Pop a message (Consumer)
    std::optional<Response> pop() {
        if (tail == head) {
            // Buffer is empty
            return std::nullopt;
        }

        Response message = buffer[tail].load(std::memory_order_acquire);
        tail = (tail + 1) % buffer.size();
        return message;
    }

  private:
    std::vector<std::atomic<Response>> buffer;
    size_t head;
    size_t tail;
};
