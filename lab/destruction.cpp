#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// Dummy Unit class representing the object to be deleted
class Unit {
  public:
    Unit(int id) : id_(id) {}
    ~Unit() { std::cout << "Unit " << id_ << " destroyed.\n"; }

  public:
    int id_;
};

// Lock-free single-producer single-consumer queue
class SPSCQueue {
  public:
    SPSCQueue(size_t capacity) : buffer_(capacity), head_(0), tail_(0) {}

    bool push(Unit *unit) {
        size_t next_head = (head_ + 1) % buffer_.size();
        if (next_head == tail_.load(std::memory_order_acquire)) {
            // Queue is full
            return false;
        }
        buffer_[head_] = unit;
        head_ = next_head;
        return true;
    }

    Unit *pop() {
        if (tail_.load(std::memory_order_acquire) == head_) {
            // Queue is empty
            return nullptr;
        }
        Unit *unit = buffer_[tail_];
        buffer_[tail_] = nullptr;
        tail_ = (tail_ + 1) % buffer_.size();
        return unit;
    }

    bool isEmpty() const {
        return tail_.load(std::memory_order_acquire) == head_;
    }

  private:
    std::vector<Unit *> buffer_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

// PlayerEngine class simulating the audio thread
class PlayerEngine {
  public:
    PlayerEngine(SPSCQueue &queue, std::condition_variable &cv, std::atomic<bool> &process_flag)
        : queue_(queue), cv_(cv), process_flag_(process_flag) {}

    ~PlayerEngine() {
        if (currUnit != nullptr) {
            std::cout << "PlayerEngine destructor: Deleting active Unit.\n";
            delete currUnit;
            currUnit = nullptr;
        }
    }
    void run() {
        // do stuff
        if (wakeUpDestrucorOnLeave) {
            // Indicate that all Units have been queued, signal Destructor thread
            process_flag_ = true;
            cv_.notify_one();
        }
    }

    void updateUnit(Unit *newUnit) {
        if (currUnit != nullptr) {
            queue_.push(currUnit);
            wakeUpDestrucorOnLeave = true;
        }
        currUnit = newUnit;
    }

  private:
    Unit *currUnit = nullptr;
    SPSCQueue &queue_;
    std::condition_variable &cv_;
    std::atomic<bool> &process_flag_;
    bool wakeUpDestrucorOnLeave = 0;
};

// Destructor class simulating the queue-reader thread
class Destructor {
  public:
    Destructor(SPSCQueue &queue, std::condition_variable &cv, std::atomic<bool> &process_flag)
        : queue_(queue), cv_(cv), process_flag_(process_flag) {}

    void run() {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait for the signal from the PlayerEngine
        while (true) {
            cv_.wait(lock, [&] { return process_flag_.load() || !queue_.isEmpty(); });

            // Process all items in the queue
            while (!queue_.isEmpty()) {
                Unit *unit = queue_.pop();
                if (unit) {
                    delete unit; // Perform manual deletion
                    std::cout << "Destructor thread: Destroying a Unit.\n";
                }
            }

            // Exit the loop if PlayerEngine has finished and the queue is empty
            if (process_flag_.load() && queue_.isEmpty()) {
                break;
            }
        }

        std::cout << "Destructor thread: Exiting.\n";
    }

  private:
    SPSCQueue &queue_;
    std::condition_variable &cv_;
    std::atomic<bool> &process_flag_;
    std::mutex mutex_;
};

int main() {
    constexpr size_t queue_capacity = 16;
    SPSCQueue queue(queue_capacity);
    std::condition_variable cv;
    std::atomic<bool> process_flag(false);

    PlayerEngine playerEngine(queue, cv, process_flag);
    Destructor destructor(queue, cv, process_flag);

    // Launch PlayerEngine in the audio thread
    std::thread audioThread(&PlayerEngine::run, &playerEngine);

    // faking here that audioThread picks up object setup by factory.
    Unit *unit1 = new Unit(12);
    playerEngine.updateUnit(unit1);
    Unit *unit2 = new Unit(13);
    playerEngine.updateUnit(unit2);
    Unit *unit3 = new Unit(14);
    playerEngine.updateUnit(unit3);
    // perform run - destructor shold be started..
    playerEngine.run();

    // Launch Destructor in the queue-reader thread
    std::thread destructorThread(&Destructor::run, &destructor);

    // Join threads
    audioThread.join();
    destructorThread.join();

    return 0;
}
