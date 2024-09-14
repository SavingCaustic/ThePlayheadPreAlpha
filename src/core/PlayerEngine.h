#pragma once

#include "Rack.h"
#include "Rotator.h"
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

// Forward declaration of Rack
class Rack;

class PlayerEngine {
  public:
    static const std::size_t MAX_RACKS = TPH_RACK_COUNT;

    PlayerEngine();

    void reset();
    void doReset();

    bool setupRackWithSynth(int rackId, const std::string &synthName);
    bool loadRack(std::unique_ptr<Rack> rack, std::size_t position);
    Rack *getRack(std::size_t position) const;
    void render(float *buffer, unsigned long numFrames);
    void noteOnDemo();
    void renderNextBlock(float *buffer, unsigned long numFrames);

  private:
    void clockResetMethod();
    bool pollMidiIn();
    void turnRackAndRender();
    void sumToMaster(float *buffer, int outer);

    std::array<std::unique_ptr<Rack>, MAX_RACKS> racks; // Fixed-size array of unique pointers to racks
    float noiseVolume;
    Rotator hRotator;        // Rotator object
    bool clockReset = false; // Clock reset flag
    bool isPlaying = false;  // Indicates if the player is currently playing
};
