#pragma once
#include <atomic>
#include <cstring> // For strcpy
#include <iostream>
#include <optional>
#include <vector>

// Fixed-size string lengths for the Message struct
constexpr size_t kTargetSize = 10;
constexpr size_t kParamNameSize = 16;

// A message struct with fixed-size strings
struct Message {
    int rackId;
    char target[kTargetSize];       // Fixed-size char array for target
    char paramName[kParamNameSize]; // Fixed-size char array for paramName
    int paramValue;

    // Default constructor (for default initialization)
    Message() : rackId(0), paramValue(0) {
        std::fill(std::begin(target), std::end(target), '\0');       // Initialize target with empty string
        std::fill(std::begin(paramName), std::end(paramName), '\0'); // Initialize paramName with empty string
    }

    // Constructor to initialize the Message struct with values
    Message(int rackId_, const char *target_, const char *paramName_, int paramValue_)
        : rackId(rackId_), paramValue(paramValue_) {
        strncpy(target, target_, kTargetSize - 1);
        target[kTargetSize - 1] = '\0'; // Ensure null termination
        strncpy(paramName, paramName_, kParamNameSize - 1);
        paramName[kParamNameSize - 1] = '\0'; // Ensure null termination
    }
};

class MessageReciever {
  public:
    explicit MessageReciever(size_t bufferSize)
        : buffer(bufferSize), head(0), tail(0) {}

    // Push a message (Producer)
    bool push(const Message &message) {
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
    std::optional<Message> pop() {
        if (tail == head) {
            // Buffer is empty
            return std::nullopt;
        }

        Message message = buffer[tail].load(std::memory_order_acquire);
        tail = (tail + 1) % buffer.size();
        return message;
    }

  private:
    std::vector<std::atomic<Message>> buffer;
    size_t head;
    size_t tail;
};

// Example usage
/*
int main() {
    MessageRouter router(10); // Buffer size of 10

    // Producer: push a message
    Message msg(1, "synth", "volume", 75);
    if (router.push(msg)) {
        std::cout << "Message pushed successfully" << std::endl;
    } else {
        std::cout << "Message buffer is full" << std::endl;
    }

    // Consumer: pop a message
    auto poppedMsg = router.pop();
    if (poppedMsg) {
        std::cout << "Popped message: " << poppedMsg->paramName << " = " << poppedMsg->paramValue << std::endl;
    } else {
        std::cout << "No messages in the buffer" << std::endl;
    }

    return 0;
}
*/