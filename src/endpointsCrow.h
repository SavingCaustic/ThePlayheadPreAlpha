#pragma once
#include "crow.h" // Crow server library
#include <crow/json.h> // Ensure to include this

// Forward declarations of your global/static classes
class PlayerEngine;
class AudioDriver;
class MidiDriver;
class MessageInBuffer;
class MessageOutBuffer;
class MessageOutReader;
class ErrorBuffer;

// Function to set up Crow routes
void crowSetupEndpoints(
    crow::SimpleApp &api,
    PlayerEngine &playerEngine,
    AudioDriver &audioDriver,
    MidiDriver &midiDriver,
    MessageInBuffer &messageInBuffer,
    MessageOutBuffer &MessageOutBuffer,
    MessageOutReader &MessageOutReader,
    ErrorBuffer &ErrorBuffer);
