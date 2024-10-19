#pragma once
#include <cstddef>
#include <cstdint>

namespace utils {
namespace hash {
// FNV-1a constants
constexpr uint32_t FNV_prime = 16777619u;
constexpr uint32_t FNV_offset_basis = 2166136261u;

// Recursive constexpr FNV-1a hash function
constexpr uint32_t fnv1a_hash(const char *str, std::size_t length, uint32_t hash = FNV_offset_basis) {
    return length == 0 ? hash : fnv1a_hash(str + 1, length - 1, (hash ^ static_cast<uint32_t>(*str)) * FNV_prime);
}

// Helper function to call fnv1a_hash with a string literal at compile-time
constexpr uint32_t fnv1a_hash(const char *str) {
    std::size_t length = 0;
    while (str[length] != '\0')
        ++length; // Calculate length of the string
    return fnv1a_hash(str, length);
}

// Runtime version of FNV-1a hash function
uint32_t fnv1a(const char *str) {
    uint32_t hash = FNV_offset_basis;

    while (*str != '\0') {
        hash ^= static_cast<uint32_t>(*str);
        hash *= FNV_prime;
        ++str;
    }

    return hash;
}
} // namespace hash
} // namespace utils