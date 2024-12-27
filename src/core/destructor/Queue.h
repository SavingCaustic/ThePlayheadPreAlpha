#pragma once
#include <array>
#include <atomic>
#include <condition_variable>
#include <optional>

namespace Destructor {

constexpr int bufferSize = 16; // Must be a power of 2

struct Record {
    void *ptr = nullptr;     // Pointer to the object to be destroyed
    void (*deleter)(void *); // Function pointer to the deleter
};

class Queue {
  public:
    Queue() : wrIdx(0), rdIdx(0) {}

    // Push a message (Producer)
    bool push(const Record &destRec) {
        size_t nextWrIdx = (wrIdx.load(std::memory_order_relaxed) + 1) & (bufferSize - 1);
        if (nextWrIdx == rdIdx.load(std::memory_order_acquire)) {
            // Buffer is full
            return false;
        }

        destructorQueue[wrIdx.load(std::memory_order_relaxed)] = destRec;
        wrIdx.store(nextWrIdx, std::memory_order_release); // Atomically update wrIdx
        cv_.notify_one();                                  // Notify the worker
        return true;
    }

    // Pop a message (Consumer)
    std::optional<Record> pop() {
        size_t currentRdIdx = rdIdx.load(std::memory_order_acquire);
        if (currentRdIdx == wrIdx) {
            // Buffer is empty
            return std::nullopt;
        }

        Record record = destructorQueue[currentRdIdx];
        rdIdx.store((currentRdIdx + 1) & (bufferSize - 1), std::memory_order_release);
        return record;
    }

    bool isEmpty() const {
        return rdIdx.load(std::memory_order_acquire) == wrIdx;
    }

    // Condition variable for synchronization
    std::condition_variable cv_;

  private:
    std::array<Record, bufferSize> destructorQueue;
    std::atomic<size_t> wrIdx; // Producer writes to head
    std::atomic<size_t> rdIdx; // Consumer reads from tail
};

} // namespace Destructor
