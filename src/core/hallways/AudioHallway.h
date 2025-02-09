#pragma once
#include "core/destructor/Queue.h"
#include "core/logger/AudioLoggerQueue.h"
#include <iostream>

class AudioHallway {
  public:
    AudioHallway() {
    }

    // Mount methods for the queues
    void destructorQueueMount(Destructor::Queue &destQueueRef) {
        destructorQueue = &destQueueRef;
    }

    void audioQueueMount(AudioLoggerQueue &alogQueue) {
        audioLoggerQueue = &alogQueue;
    }

    // Add to the destructor queue or perform immediate destruction
    bool destructorAdd(Destructor::Record &destRec) {
        if (destructorQueue) {
            return destructorQueue->push(destRec);
        } else {
            std::cout << "No destructor setup; destructing object directly." << std::endl;
            destRec.deleter(destRec.ptr); // Call the deleter function
            return true;
        }
    }

    // Log message to audioLoggerQueue or print to console if unavailable
    void logMessage(const LoggerRec &logRec) {
        if (audioLoggerQueue) {
            audioLoggerQueue->addAudioLog(logRec.code, logRec.message);
        } else {
            std::cout << "No logger: " << logRec.message << std::endl;
        }
    }

    void setMasterTune(const uint16_t newVal) {
        masterTune = newVal;
    }

  public:
    uint16_t masterTune;

  private:
    Destructor::Queue *destructorQueue = nullptr;
    AudioLoggerQueue *audioLoggerQueue = nullptr;
};

// Declare thread-local instance of AudioHallway
extern thread_local AudioHallway audioHallway;
