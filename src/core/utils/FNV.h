#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace Utils {
namespace Hash {
// FNV-1a constants
constexpr uint32_t FNV_prime = 16777619u;
constexpr uint32_t FNV_offset_basis = 2166136261u;

// Optimized compile-time FNV-1a hash function
constexpr uint32_t fnv1a_hash(const char *str, uint32_t hash = 2166136261u) {
    return (*str == '\0') ? hash : fnv1a_hash(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619u);
}

// FNV-1a hash function for std::string
uint32_t fnv1a(const std::string &str) {
    uint32_t hash = FNV_offset_basis;
    for (char c : str) {
        hash ^= static_cast<uint32_t>(c);
        hash *= FNV_prime;
    }
    return hash;
}

// Runtime version of FNV-1a hash function
uint32_t fnv1a_old(const char *str) {
    uint32_t hash = FNV_offset_basis;

    while (*str != '\0') {
        hash ^= static_cast<uint32_t>(*str);
        hash *= FNV_prime;
        ++str;
    }

    return hash;
}
} // namespace Hash
} // namespace Utils