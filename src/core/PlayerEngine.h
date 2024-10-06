#pragma once

#include "../drivers/MidiDriver.h"
#include "./messages/MessageReciever.h"
#include "./messages/MessageSender.h"
#include "Rack.h"
#include "timing/Rotator.h"
#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

class MessageReciever;
class MessageSender;
struct Message;
struct MessageOut;

class PlayerEngine {
  public:
    PlayerEngine(); // Add reference to constructor

    void BindMessageReciever(MessageReciever &hMessageReciever);
    void BindMessageSender(MessageSender &hMessageSender);
    bool sendMessage(int rackId, const char *target, float paramValue, const char *paramName, const char *paramLabel);

    void reset();
    void doReset();
    void initializeRacks();

    void ping();
    float getLoadAvg();
    void testRackSetup();

    std::string getSynthParams(int rackId);

    void midiEnable(MidiDriver *midiDriver);
    void midiDisable();
    bool setupRackWithSynth(int rackId, const std::string &synthName);
    // R    bool loadRack(std::unique_ptr<Rack> rack, std::size_t position);
    // R    Rack *getRack(std::size_t position) const;
    void renderNextBlock(float *buffer, unsigned long numFrames);
    // may be private

  private:
    Rack racks[TPH_RACK_COUNT]; // Array of Rack objects
    //  Other members...

    void clockResetMethod();
    bool pollMidiIn();
    void turnRackAndRender();
    void sumToMaster(float *buffer, unsigned long numFrames, int outer);
    // hmm.. The racks should probably be on the stack instead, to speed up buffer management
    float noiseVolume;
    Rotator hRotator;        // Rotator object
    bool clockReset = false; // Clock reset flag
    bool isPlaying = false;  // Indicates if the player is currently playing
    MidiDriver *hMidiDriver = nullptr;
    bool midiMultiMode = false; // true if midiCh should be directed to respective rack
    int rackInFocus = 0;        // not sure about this.. number of rack in focus. What if no rack in focus?
    int rackReceivingMidi = 0;
    std::vector<unsigned char> midiInMsg;
    double midiInTS; // probably not used, we use it when we get it.
    MessageReciever *messageReciever = nullptr;
    MessageSender *messageSender = nullptr;
    Message newMessage;                 // Declare a reusable Message object
    std::atomic<bool> isWritingMessage; // Atomic flag to track write access

    float loadAvg = 0;
};
