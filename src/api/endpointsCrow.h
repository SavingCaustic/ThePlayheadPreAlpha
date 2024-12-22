#pragma once
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
void crowSetupEndpoints(
    crow::SimpleApp &api,
    PlayerEngine &playerEngine,
    AudioManager &audioManager,
    MidiManager &midiManager,
    MessageInBuffer &messageInBuffer,
    MessageOutBuffer &MessageOutBuffer,
    MessageOutReader &MessageOutReader,
    ErrorBuffer &ErrorBuffer,
    RPCParser &rpcParser);
