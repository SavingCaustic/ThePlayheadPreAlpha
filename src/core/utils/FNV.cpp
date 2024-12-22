#include "core/utils/FNV.h"

namespace Utils::Hash {
uint32_t fnv1a(const std::string &str) {
    uint32_t hash = FNV_offset_basis;
    for (char c : str) {
        hash ^= static_cast<uint32_t>(c);
        hash *= FNV_prime;
    }
    return hash;
}
} // namespace Utils::Hash
