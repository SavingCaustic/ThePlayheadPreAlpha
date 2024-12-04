#pragma once
#include <array>
#include <atomic>
#include <optional>

constexpr int bufferSize = 16; // Must be a power of 2

struct DestructorRecord {
    void *ptr;               // Pointer to the object to be destroyed
    void (*deleter)(void *); // Function pointer to the deleter
};

class DestructorQueue {
  public:
    DestructorQueue() : wrIdx(0), rdIdx(0) {}

    // Push a message (Producer)
    bool push(const DestructorRecord &destRec) {
        size_t nextWrIdx = (wrIdx.load(std::memory_order_relaxed) + 1) & (bufferSize - 1);
        if (nextWrIdx == rdIdx.load(std::memory_order_acquire)) {
            // Buffer is full
            return false;
        }

        destructorQueue[wrIdx.load(std::memory_order_relaxed)] = destRec;
        wrIdx.store(nextWrIdx, std::memory_order_release); // Atomically update wrIdx
        return true;
    }

    // Pop a message (Consumer)
    std::optional<DestructorRecord> pop() {
        size_t currentRdIdx = rdIdx.load(std::memory_order_acquire);
        if (currentRdIdx == wrIdx) {
            // Buffer is empty
            return std::nullopt;
        }

        DestructorRecord record = destructorQueue[currentRdIdx];
        rdIdx.store((currentRdIdx + 1) & (bufferSize - 1), std::memory_order_release);
        return record;
    }

  private:
    std::array<DestructorRecord, bufferSize> destructorQueue;
    std::atomic<size_t> wrIdx; // Producer writes to head
    std::atomic<size_t> rdIdx; // Consumer reads from tail
};
