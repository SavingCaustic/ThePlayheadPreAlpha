#pragma once
#include <core/utils/FNV.h>
#include <iostream>
#include <string>

namespace Factory {

// factories are static
class Project {
  public:
    static void parse(u_int32_t methodFNV, std::string key, std::string strValue) {
        switch (methodFNV) {
        case Utils::Hash::fnv1a_hash("load"):
            break;
        case Utils::Hash::fnv1a_hash("save"):
            break;
        case Utils::Hash::fnv1a_hash("save_as"):
            break;
        case Utils::Hash::fnv1a_hash("delete"):
            break;
        }
    }
};
} // namespace Factory