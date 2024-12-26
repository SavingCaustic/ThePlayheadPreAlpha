#pragma once
#include "./Queue.h"
#include <array>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <optional>
#include <thread>

// Destructor class simulating the queue-reader thread
// Worker class
namespace Destructor {

class Worker {
  public:
    Worker(Queue &queue)
        : queue_(queue), running(false), destructorThread(nullptr) {}

    // Starts the thread
    void start() {
        running = true;
        std::cout << "Starting destructor..\n";
        destructorThread = std::make_unique<std::thread>(&Destructor::Worker::run, this);
    }

    // Stops the thread
    void stop() {
        running = false;
        if (destructorThread && destructorThread->joinable()) {
            std::cout << "Stopping destructor..\n";
            queue_.cv_.notify_one();  // Ensure thread wakes up if waiting
            destructorThread->join(); // Ensure the thread finishes execution
        }
    }

    void run() {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait for new items to process or for the queue to be empty
        while (running || !queue_.isEmpty()) {
            // Wait for new items or stop signal
            queue_.cv_.wait(lock, [&] { return !queue_.isEmpty() || !running; });

            // Process all items in the queue
            while (!queue_.isEmpty()) {
                auto record = queue_.pop();
                if (record) {
                    std::cout << "nam-nam destructor yammy.." << std::endl;
                    record->deleter(record->ptr); // Call the deleter function
                    std::cout << "Destructor thread: Destroying an object.\n";
                }
            }

            // Exit the loop if the queue is empty and stop is requested
            if (queue_.isEmpty() && !running) {
                break;
            }
        }

        std::cout << "Destructor thread: Exiting.\n";
    }

  private:
    std::atomic<bool> running; // Flag to control the thread's run state
    std::unique_ptr<std::thread> destructorThread;

    Queue &queue_;
    std::mutex mutex_;
};

} // namespace Destructor
