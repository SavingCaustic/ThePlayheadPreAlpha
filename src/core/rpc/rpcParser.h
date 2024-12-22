#pragma once
#include "core/factory/rack.h"
#include "core/factory/server.h"
#include "core/factory/unit.h"
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

  public:
    // Constructor with an initializer list to set the reference
    explicit RPCParser(Constructor::Queue &queue) : constructorQueue(queue) {}

    void parse(const std::string &strClass, const std::string &strMethod, const std::string &strKey,
               const std::string &strVal, const std::string &rackID) {
        // ok. for simplicity.
        uint32_t classFNV = Utils::Hash::fnv1a(strClass);
        uint32_t methodFNV = Utils::Hash::fnv1a(strMethod);
        switch (classFNV) {
        case Utils::Hash::fnv1a_hash("server"):
            std::cout << "parsing verb: " << strMethod << std::endl;
            Factory::Server::parse(methodFNV, "", "");
            break;
        case Utils::Hash::fnv1a_hash("device"):
            break;
        case Utils::Hash::fnv1a_hash("project"):
            break;
        case Utils::Hash::fnv1a_hash("rack"):
            std::cout << "rack here we go.." << std::endl;
            Factory::Rack::parse(strMethod, strKey, strVal, stoi(rackID), constructorQueue);
            break;
        case Utils::Hash::fnv1a_hash("unit"):
            std::cout << "unit here we go.." << std::endl;
            Factory::Unit::parse(strMethod, strKey, strVal, stoi(rackID), constructorQueue);
            break;
        case Utils::Hash::fnv1a_hash("pattern"):
            break;
        }
    }
};
