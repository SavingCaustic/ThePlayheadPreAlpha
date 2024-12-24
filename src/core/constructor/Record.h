#pragma once
#include <array>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <optional>

namespace Constructor {

struct Record {
    void *ptr;     // Pointer to the new
    uint32_t size; // Size of the object
    bool isStereo; // True if stereo
    char type[32]; // Fixed-size target type identifier, max 31 chars + null terminator, like synth.lut1_overtones
    int rackID;    // Yeah. Int for now..

    // Constructor for initialization
    Record(void *p, uint32_t s, bool stereo, const char *t, int rackID)
        : ptr(p), size(s), isStereo(stereo), rackID(rackID) {
        std::strncpy(type, t, sizeof(type) - 1);
        type[sizeof(type) - 1] = '\0';
        rackID = rackID;
    }

    // Default constructor
    Record() : ptr(nullptr), size(0), isStereo(false), rackID(-1) {
        std::fill(std::begin(type), std::end(type), '\0');
    }

    /*
        // Copy constructor
        Record(const Record &other)
            : ptr(other.ptr), size(other.size), isStereo(other.isStereo) {
            std::memcpy(type, other.type, sizeof(type));
        }

        // Assignment operator
        Record &operator=(const Record &other) {
            if (this != &other) {
                ptr = other.ptr;
                size = other.size;
                isStereo = other.isStereo;
                std::memcpy(type, other.type, sizeof(type));
            }
            return *this;
        }
        */
};
} // namespace Constructor