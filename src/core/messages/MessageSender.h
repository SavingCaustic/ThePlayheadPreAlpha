#pragma once
#include <atomic>
#include <condition_variable>
#include <cstring> // For strcpy
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

constexpr size_t msgOutTargetSize = 10;
constexpr size_t msgOutParamNameSize = 16;
constexpr size_t msgOutParamLabelSize = 10;

struct MessageOut {
    int rackId;
    char target[msgOutTargetSize];
    char paramName[msgOutParamNameSize];
    float paramValue;
    char paramLabel[msgOutParamLabelSize];

    MessageOut() : rackId(0), paramValue(0) {
        std::fill(std::begin(target), std::end(target), '\0');
        std::fill(std::begin(paramName), std::end(paramName), '\0');
    }

    MessageOut(int rackId_, const char *target_, const char *paramName_, float paramValue_, const char *paramLabel_)
        : rackId(rackId_), paramValue(paramValue_) {
        strncpy(target, target_, msgOutTargetSize - 1);
        target[msgOutTargetSize - 1] = '\0';
        strncpy(paramName, paramName_, msgOutParamNameSize - 1);
        paramName[msgOutParamNameSize - 1] = '\0';
    }
};

class MessageSender {
  public:
    explicit MessageSender(size_t bufferSize)
        : buffer(bufferSize), head(0), tail(0), sleeping(true) {}

    // Push a message (Producer) without using a mutex
    bool push(const MessageOut &message) {
        size_t nextHead = (head + 1) % buffer.size();
        if (nextHead == tail) {
            // Buffer is full
            return false;
        }

        buffer[head].store(message, std::memory_order_release);
        head = nextHead;

        // Only notify if the consumer thread is sleeping
        if (sleeping.load(std::memory_order_acquire)) {
            // Wake up the consumer thread
            cv.notify_one();
        }

        return true;
    }

    // Pop a message (Consumer)
    std::optional<MessageOut> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return tail != head; });

        MessageOut message = buffer[tail].load(std::memory_order_acquire);
        tail = (tail + 1) % buffer.size();
        return message;
    }

    // A non-blocking pop, without waking the thread
    std::optional<MessageOut> tryPop() {
        if (tail == head) {
            return std::nullopt;
        }

        MessageOut message = buffer[tail].load(std::memory_order_acquire);
        tail = (tail + 1) % buffer.size();
        return message;
    }

    // Consumer thread sets sleeping state before going to sleep
    void setSleeping(bool state) {
        sleeping.store(state, std::memory_order_release);
    }

  private:
    std::vector<std::atomic<MessageOut>> buffer;
    std::atomic<size_t> head;
    std::atomic<size_t> tail;

    std::atomic<bool> sleeping; // Tracks whether the consumer thread is sleeping
    std::mutex mutex;           // Protects the condition variable
    std::condition_variable cv; // Condition variable for waking the consumer thread
};
