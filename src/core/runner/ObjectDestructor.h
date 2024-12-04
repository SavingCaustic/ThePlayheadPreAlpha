#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

// Queue for destroyed objects
// this doesn't work currently, since Object is unknown and is probably the base-type unit.
// because i guess we never really destory a rack. We just blank it..

std::queue<std::shared_ptr<Object>> destroyedQueue;
std::mutex queueMutex;
std::condition_variable cv;
bool stopObjectDestructor = false;

void ManagerThread(std::shared_ptr<Object> objectToDestroy) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        destroyedQueue.push(objectToDestroy); // Push smart pointer
    }
    cv.notify_one(); // Signal the ObjectDestructor thread
}

void ObjectDestructor() {
    while (!stopObjectDestructor) {
        std::shared_ptr<Object> obj;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [] { return !destroyedQueue.empty() || stopObjectDestructor; });

            if (!destroyedQueue.empty()) {
                obj = destroyedQueue.front();
                destroyedQueue.pop();
            }
        } // Unlocks automatically at the end of the scope

        if (obj) {
            // Perform cleanup tasks
            CleanUp(obj.get()); // Use raw pointer for cleanup if necessary
            // No need to delete obj; std::shared_ptr will take care of that
        }
    }
}

// CleanUp function
void CleanUp(Object *obj) {
    // Perform necessary cleanup tasks here, such as removing from DSP chain
}

// Main program or entry point where threads are created
