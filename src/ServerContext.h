// NOT USED. Should simplify main.cpp or actually ThePlayhead as main gets minimized..
#pragma once

// #include "ThePlayhead.h" // Include the playhead header
#include "core/PlayerEngine.h"
#include "core/messages/MessageInBuffer.h"
#include "core/messages/MessageOutBuffer.h"
#include "core/messages/MessageOutReader.h"
#include "drivers/AudioDriver.h"
#include "drivers/MidiDriver.h"

class ServerContext {
  public:
    ServerContext(PlayerEngine &pe, AudioDriver &ad, MidiDriver &md,
                  MessageInBuffer &mib, MessageOutBuffer &mob, MessageOutReader &mor) //, ThePlayhead &tph)
        : playerEngine(pe), audioDriver(ad), midiDriver(md),
          messageInBuffer(mib), messageOutBuffer(mob), messageOutReader(mor) {} //, thePlayhead(tph) {}

    PlayerEngine &playerEngine;
    AudioDriver &audioDriver;
    MidiDriver &midiDriver;
    MessageInBuffer &messageInBuffer;
    MessageOutBuffer &messageOutBuffer;
    MessageOutReader &messageOutReader;
    // ThePlayhead &thePlayhead; // Add the playhead here
};
