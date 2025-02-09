#pragma once
#include "core/factory/constructor/Queue.h"
#include "core/logger/LoggerQueue.h"
#include "core/parameters/ProjectSettingsManager.h"
#include "core/storage/DataStore.h"
#include <iostream>

class FactoryHallway {
    // since we want testing, late binding is the key here..
    // logger, constructor and destructor
    // logger = error before refactor.
  public:
    void constructorQueueMount(Constructor::Queue &constrQueueRef) {
        constructorQueue = &constrQueueRef;
    }

    void loggerMount(LoggerQueue &logQueue) {
        loggerQueue = &logQueue;
    }

    void datastoreMount(Storage::DataStore &dStore) {
        dataStore = &dStore;
    }

    void projectSettingsMount(ProjectSettingsManager &psManager) {
        projectSettingsManager = &psManager;
    }

    bool constructorPush(void *ptr, uint32_t size, bool isStereo, const char *type, int rackID) {
        if (constructorQueue) {
            constructorQueue->push(ptr, size, isStereo, type, rackID);
            // return constructorQueue->push(destRec);
            return true;
        } else {
            std::cout << "no constructor setup so caller must destruct!" << std::endl;
            // delete ptr;
            return false;
        }
    }

    void logMessage(LoggerRec &logRec) {
        if (loggerQueue) {
            // mutex provided by loggerBuffer..
            loggerQueue->write(logRec);
        } else {
            std::cout << "No logger present but here's the message: " << logRec.message << std::endl;
        }
    }

    Storage::DataStore &storeGetRef() {
        // called from static classes. Maybe provide ref to hallway instead..
        return *dataStore;
    }

    void storeLoadProject(const std::string &projectName) {
        // this should go but i'm not there..
        dataStore->projectLoad(projectName);
    }

    void test() {
        // do nothing
    }

    /*
    ProjectSettingsManager &psGetRef() {
        // called from static classes. Maybe provide ref to hallway instead..
        return *projectSettingsManager;
    }
    */

    void pushProjectSetting(const std::string &key, const std::string &value) {
        std::cout << "picabo2" << std::endl;
        projectSettingsManager->setSetting(key, value);
    }

  private:
    Constructor::Queue *constructorQueue = nullptr;
    LoggerQueue *loggerQueue = nullptr;
    Storage::DataStore *dataStore = nullptr;
    ProjectSettingsManager *projectSettingsManager = nullptr;
};

extern FactoryHallway factoryHallway;
