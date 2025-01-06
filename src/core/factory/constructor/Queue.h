#pragma once

#include "./Record.h"
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <optional>

namespace Constructor {
// note that constructor has no reader since playerEngine is polling.

constexpr int bufferSize = 16; // Must be a power of 2

class Queue {
  public:
    Queue() : wrIdx(0), rdIdx(0) {}
    explicit Queue(size_t) : wrIdx(0), rdIdx(0) {
        // Optionally accept a size parameter, but currently unused
    }

    bool push(void *ptr, uint32_t size, bool isStereo, const char *type, int rackID) {
        size_t nextWrIdx = (wrIdx.load(std::memory_order_relaxed) + 1) & (bufferSize - 1);
        if (nextWrIdx == rdIdx.load(std::memory_order_acquire)) {
            // Buffer is full
            return false;
        }

        // Construct a Record and add it to the queue
        constructorQueue[wrIdx.load(std::memory_order_relaxed)] = Record(ptr, size, isStereo, type, rackID);
        wrIdx.store(nextWrIdx, std::memory_order_release); // Atomically update wrIdx
        return true;
    }

    // Pop a message (Consumer)
    std::optional<Record> pop() {
        size_t currentRdIdx = rdIdx.load(std::memory_order_acquire);
        if (currentRdIdx == wrIdx) {
            // Buffer is empty
            return std::nullopt;
        }

        Record record = constructorQueue[currentRdIdx];
        rdIdx.store((currentRdIdx + 1) & (bufferSize - 1), std::memory_order_release);
        return record;
    }

    bool isEmpty() const {
        return rdIdx.load(std::memory_order_acquire) == wrIdx;
    }

  private:
    std::array<Record, bufferSize> constructorQueue;
    std::atomic<size_t> wrIdx; // Producer writes to head
    std::atomic<size_t> rdIdx; // Consumer reads from tail
};

} // namespace Constructor
