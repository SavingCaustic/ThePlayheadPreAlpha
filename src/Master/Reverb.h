#include <atomic>
#include <condition_variable>
#include <constants.h>
#include <cstdlib> // For rand()
#include <ctime>   // For seeding the random number generator
#include <mutex>
#include <thread>

class MasterReverb {

  public:
    void reverbThread() {
        std::unique_lock<std::mutex> dummyLock(mutex); // Required for condition_variable_any

        while (running) {
            // Wait for a signal
            reverbCV.wait(dummyLock, [this]() { return isReverbDataReady(); });
            // Process reverb
            processReverb(audioToReverbBuffer, reverbOutputBuffer);

            // Reset the flag
            reverbDataReady.store(false, std::memory_order_release);
        }
    }

    bool isReverbDataReady() {
        return reverbDataReady.load(std::memory_order_acquire);
    }

    void processReverb(float in[], float out[]) {
        static bool seeded = false;
        if (!seeded) {
            std::srand(static_cast<unsigned>(std::time(nullptr))); // Seed the RNG
            seeded = true;
        }

        for (size_t i = 0; i < TPH_RACK_RENDER_SIZE; ++i) {
            out[i] = (static_cast<float>(std::rand()) / RAND_MAX) * 0.2f - 0.1f; // Random float in [+/- 0.1]
        }
    }

  public:                                      //??
    std::atomic<bool> reverbDataReady = false; // Atomic flag for data readiness
    std::condition_variable_any reverbCV;      // Condition variable
    std::atomic<bool> running = true;          // Controls the threads
    std::mutex mutex;                          // Dummy mutex for condition_variable_any

    float audioToReverbBuffer[TPH_RACK_RENDER_SIZE]; // Shared buffer
    float reverbOutputBuffer[TPH_RACK_RENDER_SIZE];  // Shared buffer
};

/* audio thread example
void audioThread() {
    while (running) {
        // Mix main audio and write to buffer
        mixMainAudio(audioToReverbBuffer);

        // Signal the reverb-thread
        reverbDataReady.store(true, std::memory_order_release);
        reverbCV.notify_one();

        // Optionally, read processed reverb data
            addToMix(reverbOutputBuffer);
        }
    }
}
*/