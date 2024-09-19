#pragma once

#include "../drivers/MidiDriver.h"
#include "MessageRouter.h"
#include "Rack.h"
#include "timing/Rotator.h"
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

// Forward declaration of Rack
class Rack;

class MessageRouter;
struct Message;

class PlayerEngine {
  public:
    static const std::size_t MAX_RACKS = TPH_RACK_COUNT;

    // PlayerEngine(MessageRouter &hMessageRouter); // Add reference to constructor
    PlayerEngine(); // Add reference to constructor
    void BindMessageRouter(MessageRouter &hMessageRouter);

    void reset();
    void doReset();
    void ping();
    void testRackSetup();

    void midiEnable(MidiDriver *midiDriver);
    void midiDisable();
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
    void sumToMaster(float *buffer, unsigned long numFrames, int outer);

    std::array<std::unique_ptr<Rack>, MAX_RACKS> racks; // Fixed-size array of unique pointers to racks
    float noiseVolume;
    Rotator hRotator;        // Rotator object
    bool clockReset = false; // Clock reset flag
    bool isPlaying = false;  // Indicates if the player is currently playing
    MidiDriver *hMidiDriver = nullptr;
    bool midiMultiMode = false; // true if midiCh should be directed to respective rack
    int rackInFocus = 0;        // number of rack in focus. What if no rack in focus?
    std::vector<unsigned char> midiInMsg;
    double midiInTS; // probably not used, we use it when we get it.
    MessageRouter *messageRouter = nullptr;
    Message newMessage; // Declare a reusable Message object
};
