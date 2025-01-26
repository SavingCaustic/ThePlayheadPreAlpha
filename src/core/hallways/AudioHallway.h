#pragma once
#include "core/destructor/Queue.h"
#include "core/logger/AudioLoggerQueue.h"
#include <iostream>

class AudioHallway {
    /* THIS SHOULD NOT BE USED. Audio thread may be forked for pre-rendering */
  public:
    AudioHallway(Destructor::Queue *destrRef = nullptr, AudioLoggerQueue *auQueue)
        : destructorQueue(destrRef), audioLoggerQueue(auQueue) {
        std::cerr << "audioHallway should not be used" << std::endl;
    }

    void destructorQueueMount(Destructor::Queue &destQueueRef) {
        //?? used
        destructorQueue = &destQueueRef;
    }

    void audioQueueMount(AudioLoggerQueue &alogQueue) {
        //?? used - rename all queues to queues - not buffers.
        audioLoggerQueue = &alogQueue;
    }

    bool destructorAdd(Destructor::Record &destRec) {
        if (destructorQueue) {
            return destructorQueue->push(destRec);
        } else {
            std::cout << "no destructor setup so destructing object here" << std::endl;
            destRec.deleter(destRec.ptr); // Call the deleter function
            return true;
        }
    }

    void logMessage(LoggerRec &logRec) {
        if (audioLoggerQueue) {
            audioLoggerQueue->addAudioLog(logRec.code, logRec.message);
        } else {
            std::cout << "No logger present but here's the message: " << logRec.message << std::endl;
        }
    }

  private:
    Destructor::Queue *destructorQueue = nullptr;
    AudioLoggerQueue *audioLoggerQueue = nullptr;
};

extern AudioHallway audioHallway;
