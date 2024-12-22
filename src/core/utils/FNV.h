#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace Utils::Hash {
// FNV-1a constants
constexpr uint32_t FNV_prime = 16777619u;
constexpr uint32_t FNV_offset_basis = 2166136261u;

// Optimized compile-time FNV-1a hash function
constexpr uint32_t fnv1a_hash(const char *str, uint32_t hash = 2166136261u) {
    return (*str == '\0') ? hash : fnv1a_hash(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619u);
}

// FNV-1a hash function for std::string
uint32_t fnv1a(const std::string &str);
} // namespace Utils::Hash
