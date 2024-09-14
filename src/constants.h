#pragma once

// some constants that's global, for now..
extern const int TPH_AUDIO_SR;
extern const int TPH_AUDIO_BUFFER_SIZE;
extern const int TPH_DSP_SR;          // curr not impl
extern const int TPH_TICKS_PER_CLOCK; // PPQN = 192

constexpr int TPH_RACK_COUNT = 4; // keep small for easy debugging..
constexpr int TPH_RACK_RENDER_SIZE = 64;
