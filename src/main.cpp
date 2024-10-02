#include "constants.h"
#include "core/ErrorLogger.h"
#include "core/MessageReciever.h"
#include "core/PlayerEngine.h"
#include "core/Rack.h"
#include "core/audio/AudioMath.h"
#include "crow.h"
#include "drivers/AudioDriver.h"
#include "drivers/FileDriver.h"
#include "drivers/MidiDriver.h"
#include "endpointsCrow.h"
#include <atomic>
#include <chrono>
#include <filesystem> // C++17 for checking if files exist
#include <iostream>
#include <thread>

/*      ### # # ###   ##  #    #  # # # # ###  #  ##
         #  ### ##    ### #   ###  #  ### ##  ### # #
         #  # # ###   #   ### # #  #  # # ### # # ##     */

#define DEBUG_MODE 1

// Define the global shutdown flag. Set by endpoint shutdown (not signals)
std::atomic<bool> shutdown_flag(false);

// Custom signal handler
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Caught SIGINT (Ctrl+C), setting shutdown flag..." << std::endl;
        shutdown_flag = true;
    }
}

// Define and initialize the static variables outside the class
int AudioDriver::gNumNoInputs = 0;
double AudioDriver::vol = 0.7;

// Global instances of drivers and main objects

// Rack sRacks[TPH_RACK_COUNT];
PlayerEngine sPlayerEngine; // Global pointer to PlayerEngine
MessageReciever sMessageReciever(8);
AudioDriver sAudioDriver(&sPlayerEngine);
MidiDriver sMidiDriver;

// Entry point of the program
int main() {
    sPlayerEngine.BindMessageReciever(sMessageReciever);
    AudioMath::generateLUT(); // sets up a sine lookup table of 1024 elements.
    // Create the object
    crow::SimpleApp api;
    // Remove default signal handler
    api.signal_clear();
    // Add our custom handler
    signal(SIGINT, signal_handler);
    crowSetupEndpoints(api, sPlayerEngine, sAudioDriver, sMidiDriver, sMessageReciever);
    std::thread server_thread([&api]() { api.port(18080).run(); });
    while (!shutdown_flag.load()) {
        if (DEBUG_MODE) {
            std::cout << "housekeeping.. " << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    std::cout << "Stopping the server..." << std::endl;
    api.stop();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (server_thread.joinable()) {
        server_thread.join();
    }
    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
