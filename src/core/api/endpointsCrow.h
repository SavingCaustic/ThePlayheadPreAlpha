#pragma once
#include "core/factory/constructor/Queue.h"
#include "crow.h"      // Crow server library
#include <crow/json.h> // Ensure to include this
// Forward declarations of your global/static classes
class PlayerEngine;
class AudioManager;
class MidiManager;
class MessageInBuffer;
class MessageOutBuffer;
class MessageOutReader;
class ErrorBuffer;
class RPCParser;

// Function to set up Crow routes
// Most of these dependencies shoud be pushed to the RPC-parser. Making the web-server more agnostic.

void crowSetupEndpoints(
    crow::SimpleApp &api,
    PlayerEngine &playerEngine,
    AudioManager &audioManager,
    MidiManager &midiManager,
    MessageInBuffer &messageInBuffer,
    MessageOutBuffer &MessageOutBuffer,
    MessageOutReader &MessageOutReader,
    ErrorBuffer &ErrorBuffer,
    RPCParser &rpcParser,
    Constructor::Queue &constructorQueue);
