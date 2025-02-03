#include "constants.h"

// Define the global constants here
// some constants that's global, for now..
const int TPH_AUDIO_SR = 48000;
const int TPH_AUDIO_BUFFER_SIZE = 64; // stereo-pairs..
// const int TPH_DSP_SR = 48000;         // curr not impl but could be /2 of audio_sr
const int TPH_TICKS_PER_CLOCK = 10; // PPQN = 240

// Path constants for file operations
const char *const ASSETS_DIRECTORY = "assets";
const char *const USER_DIRECTORY = "user";
