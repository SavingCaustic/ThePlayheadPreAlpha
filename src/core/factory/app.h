#pragma once
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

// factories are static
class App {
  public:
    static void parse(u_int32_t methodFNV, std::string key, std::string strValue) {
        switch (methodFNV) {
        case Utils::Hash::fnv1a_hash("quit"):
            std::cout << "shuttn down!" << std::endl;
            shutdown_flag.store(true); // Set shutdown flag to true when /shutdown is hit
            break;
        case Utils::Hash::fnv1a_hash("echo"):
            std::cout << "heellooo!" << std::endl;
            break;
        }
    }
};
} // namespace Factory