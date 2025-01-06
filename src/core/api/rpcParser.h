#pragma once
#include "core/factory/project.h"
#include "core/factory/rack.h"
#include "core/factory/server.h"
#include "core/factory/unit.h"
#include "core/storage/DataStore.h"
// #include "core/storage/Project.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

// Forward declaration
namespace Constructor {
class Queue;
}

class RPCParser {
    // Reference to Constructor::Queue
    Constructor::Queue &constructorQueue;
    Storage::DataStore &dataStore;

  public:
    // Constructor with an initializer list to set the reference
    explicit RPCParser(Constructor::Queue &queue, Storage::DataStore &store);

    void parse(const std::string &strClass, const std::string &strMethod,
               const std::string &strKey, const std::string &strVal,
               const std::string &rackID, const std::string &unit);
};
