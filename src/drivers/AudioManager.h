#pragma once
#include "AudioDriver.h"
#include "constants.h"
#include <iostream>
#include <memory>
#include <string>

// this class contain audio-in-buffers. Always available for the DSP-components.
// note that, unlike midi, only *one* audio-device can be mounted.

// since in-signals often are 1 channel, or routed two ways, it's not interleaved.

class AudioManager {
  public:
    // audioInCallback()
    // somehow the driver should call this function on audioIn.
    // and this function should know if it's mono or stereo and separate them into their buffers

    void reset() {
        // make buffers sirent.
        // maybe we're not charge over audioInRunning..
    }

    void audioInEnable() {}

    void audioInDisable() {}

  private:
    float buffer1[TPH_RACK_RENDER_SIZE]{0};
    float buffer2[TPH_RACK_RENDER_SIZE]{0};
    bool audioInRunning;
    bool audioIn2chan;
    AudioDriver audioDriver;
};