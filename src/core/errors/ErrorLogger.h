#include <array>
#include <atomic>
#include <cstring>
#include <fstream>
#include <iostream>

// i don't think this is used anymore..

namespace Logging {
class ErrorLogger {
  public:
    static constexpr size_t BufferSize = 64;
    static constexpr size_t MessageSize = 80;

    // ErrorLogger() : writeIndex(0), readIndex(0) {}

    // Method to push messages into the buffer
    static bool write(const char *message) {
        size_t current_write = writeIndex.load(std::memory_order_relaxed);
        size_t next_write = (current_write + 1) % BufferSize;

        // Check if buffer is full
        if (next_write == readIndex.load(std::memory_order_acquire)) {
            return false; // Buffer is full
        }

        // Copy message into the buffer
        std::strncpy(buffer[current_write], message, MessageSize - 1);
        buffer[current_write][MessageSize - 1] = '\0'; // Ensure null termination

        writeIndex.store(next_write, std::memory_order_release);
        return true;
    }

    // Method to pop messages from the buffer
    static bool read(char *message) {
        size_t current_read = readIndex.load(std::memory_order_relaxed);

        // Check if buffer is empty
        if (current_read == writeIndex.load(std::memory_order_acquire)) {
            return false; // Buffer is empty
        }

        // Copy the message from the buffer
        std::strncpy(message, buffer[current_read], MessageSize);

        readIndex.store((current_read + 1) % BufferSize, std::memory_order_release);
        return true;
    }

    static void flushToFile() {
        std::ifstream checkFile("error.log");
        if (!checkFile.good()) {
            std::ofstream createFile("error.log"); // This creates the file if it doesn't exist
        }
        checkFile.close();

        std::ofstream logFile("error.log", std::ios::app); // Open file in append mode
        if (!logFile) {
            std::cerr << "Failed to open log file!" << std::endl;
            return;
        }

        char message[MessageSize];
        while (read(message)) {
            logFile << message << std::endl;
        }

        logFile.close();
    }

  private:
    static std::array<char[MessageSize], BufferSize> buffer; // Fixed-size buffer for log messages
    static std::atomic<size_t> writeIndex;
    static std::atomic<size_t> readIndex;
};
} // namespace Logging