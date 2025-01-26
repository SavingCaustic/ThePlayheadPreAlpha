#include <chrono>
#include <iostream>
#include <thread>

#define SLEEP_TIME 500

// evalate calculations and thread-wakeup time.
// crucial for determining time left in render-thread.

int main() {
    // Capture the start time
    auto start_time = std::chrono::high_resolution_clock::now();

    // Simulate processing or just sleep
    std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_TIME));

    // Capture the end time
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time in microseconds
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    // Print elapsed time in microseconds
    std::cout << "Elapsed time: " << elapsed_time << " microseconds" << std::endl;
    std::cout << "Requested sleep time: " << SLEEP_TIME << " microseconds" << std::endl;

    // Estimate time left in render window (1333 microseconds)
    int64_t render_window_us = 1333;
    int64_t time_left = render_window_us - elapsed_time;

    if (time_left > 0) {
        std::cout << "Time left for processing: " << time_left << " microseconds" << std::endl;
    } else {
        std::cout << "No time left in the render window!" << std::endl;
    }

    return 0;
}