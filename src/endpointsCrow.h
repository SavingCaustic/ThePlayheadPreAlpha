#pragma once
#include "crow.h" // Crow server library

// Forward declarations of your global/static classes
class PlayerEngine;
class AudioDriver;
class MidiDriver;
class MessageReciever;

// Function to set up Crow routes
void crowSetupEndpoints(crow::SimpleApp &api, PlayerEngine &playerEngine, AudioDriver &audioDriver, MidiDriver &midiDriver, MessageReciever &messageReciever);
