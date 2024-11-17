#pragma once
#include <atomic>

// some constants that's global, for now..
extern const int TPH_AUDIO_SR;
extern const int TPH_AUDIO_BUFFER_SIZE;
extern const int TPH_DSP_SR;          // curr not impl
extern const int TPH_TICKS_PER_CLOCK; // PPQN = 192

// Path constants for file operations
extern const char *const ASSETS_DIRECTORY;
extern const char *const USER_DIRECTORY;

constexpr int TPH_RACK_COUNT = 4; // keep small for easy debugging..
constexpr int TPH_RACK_RENDER_SIZE = 64;
constexpr int TPH_RACK_BUFFER_SIZE = 128; // x2 since stereo
constexpr int LUT_SIZE = 16384;

// Declare the shutdown flag as an external variable
extern std::atomic<bool> shutdown_flag;
