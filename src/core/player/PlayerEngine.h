#pragma once

#include "CCManager/CCManager.h"
#include "ErrorWriter.h"
#include "ObjectManager/ObjectManager.h"
#include "Synth/SynthBase.h"
#include "chrono"
#include "core/destructor/Queue.h"
#include "core/errors/AudioErrorBuffer.h"
#include "core/messages/MessageInBuffer.h"
#include "core/messages/MessageOutBuffer.h"
#include "core/player/Rack/Rack.h"
#include "core/timing/Rotator.h"
#include "drivers/MidiManager.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <thread>

class MessageInBuffer;
class MessageOutBuffer;
struct MessageIn;
struct MessageOut;

class PlayerEngine {
  public:
    PlayerEngine(); // Add reference to constructor

    void bindErrorWriter();

    void bindMessageInBuffer(MessageInBuffer &hMessageInBuffer);
    void bindMessageOutBuffer(MessageOutBuffer &hMessageOutBuffer);
    void bindErrorBuffer(AudioErrorBuffer &hAudioErrorBuffer);
    void bindDestructorBuffer(Destructor::Queue &hDestructorBuffer);

    void bindMidiManager(MidiManager &hMidiManager);

    void bindConstructorQueue(Constructor::Queue &hConstructorQueue);

    bool sendMessage(int rackId, const char *target, float paramValue, const char *paramName, const char *paramLabel);

    void reset();
    void doReset();
    void initializeRacks();

    void ping();
    float getLoadAvg();

    std::string getSynthParams(int rackId);

    bool loadSynth(SynthBase *&synth, int rackID);
    bool loadEffect(EffectBase *&effect, int rackID, int effectSlot);

    bool setupRackWithSynth(int rackId, const std::string &synthName);
    // R    bool loadRack(std::unique_ptr<Rack> rack, std::size_t position);
    // R    Rack *getRack(std::size_t position) const;
    void renderNextBlock(float *buffer, unsigned long numFrames);
    int64_t sendLoadStats(std::chrono::time_point<std::chrono::high_resolution_clock> nextFrameCount, int64_t frameDurationMicroSec);

    // may be private
    void sendError(int code, const std::string &message);

    CCManager ccManager;
    ObjectManager objectManager;
    // Destructor::Queue destructorQueue;

  private:
    Rack racks[TPH_RACK_COUNT]; // Array of Rack objects
    ErrorWriter errorWriter_;   // No longer a pointer, now stack-allocated
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
    int rackReceivingMidi = -1;
    std::vector<unsigned char> midiInMsg;
    double midiInTS; // probably not used, we use it when we get it.
    MessageInBuffer *messageInBuffer = nullptr;
    MessageOutBuffer *messageOutBuffer = nullptr;
    // Destructor::Queue *destructorBuffer = nullptr;
    MessageIn newMessage;               // Declare a reusable Message object
    std::atomic<bool> isWritingMessage; // Atomic flag to track write access
    AudioErrorBuffer *audioErrorBuffer = nullptr;
    MidiManager *midiManager = nullptr;

    float loadAvg = 0;
    int debugCnt = 0;
};
