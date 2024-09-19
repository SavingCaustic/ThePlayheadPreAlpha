#pragma once
#include <atomic>
#include <iostream>
#include <optional>
#include <vector>

// A simple message struct for demo purposes
struct Message {
    int rackId;
    std::string target;
    std::string paramName;
    float paramValue;
};

class MessageRouter {
  public:
    explicit MessageRouter(size_t bufferSize)
        : buffer(bufferSize), head(0), tail(0) {
        for (auto &slot : buffer) {
            slot.store(nullptr); // Initialize atomic pointers to nullptr
        }
    }

    // Push a message (Producer)
    bool push(Message *message) {
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
    std::optional<Message *> pop() {
        if (tail == head) {
            // Buffer is empty
            return std::nullopt;
        }

        Message *message = buffer[tail].load(std::memory_order_acquire);
        buffer[tail].store(nullptr, std::memory_order_release); // Clear the slot
        tail = (tail + 1) % buffer.size();
        return message;
    }

  private:
    std::vector<std::atomic<Message *>> buffer;
    size_t head;
    size_t tail;
};

/*
// Example usage
int main() {
    MessageRouter router(10); // Buffer size of 10

    // Producer: push a message
    Message* msg = new Message{1, "synth", "volume", "0.75"};
    if (router.push(msg)) {
        std::cout << "Message pushed successfully" << std::endl;
    } else {
        std::cout << "Message buffer is full" << std::endl;
    }

    // Consumer: pop a message
    auto poppedMsg = router.pop();
    if (poppedMsg) {
        std::cout << "Popped message: " << (*poppedMsg)->paramName << " = " << (*poppedMsg)->paramValue << std::endl;
        delete *poppedMsg; // Don't forget to delete the message!
    } else {
        std::cout << "No messages in the buffer" << std::endl;
    }

    return 0;
}
*/