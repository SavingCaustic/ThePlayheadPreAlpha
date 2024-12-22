#pragma once
#include "core/factory/app.h"
#include "core/factory/unit.h"
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

class RPCParser {
    // uhm - should the factories be static? Probably..
  public:
    void parse(std::string strClass, std::string strMethod, std::string strKey, std::string strVal, std::string rackID) {
        // ok. for simplicity.
        uint32_t classFNV = Utils::Hash::fnv1a(strClass);
        uint32_t methodFNV = Utils::Hash::fnv1a(strMethod);
        switch (classFNV) {
        case Utils::Hash::fnv1a_hash("app"):
            std::cout << "parsing verb: " << strMethod << std::endl;
            Factory::App::parse(methodFNV, "", "");
            break;
        case Utils::Hash::fnv1a_hash("device"):
            break;
        case Utils::Hash::fnv1a_hash("project"):
            break;
        case Utils::Hash::fnv1a_hash("rack"):
            break;
        case Utils::Hash::fnv1a_hash("unit"):
            std::cout << "unit here we go.." << std::endl;
            Factory::Unit::parse(strMethod, strKey, strVal, stoi(rackID));
            break;
        case Utils::Hash::fnv1a_hash("pattern"):
            break;
        }
    }
};