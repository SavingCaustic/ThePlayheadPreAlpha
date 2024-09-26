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

    CROW_ROUTE(api, "/")
    ([]() { return crow::response(200, "PLAYHEAD AUDIO SERVER"); });

    // namespace fs = std::filesystem;

    CROW_ROUTE(api, "/fe/<string>")
    ([](const std::string &filename) {
        std::string filepath = "assets/fe/" + filename;

        // Check if file exists
        if (std::filesystem::exists(filepath)) {
            std::string fileContent = FileDriver::load_file_content(filepath);

            // Determine file type based on extension
            std::string contentType = "text/plain";
            if (FileDriver::ends_with(filename, ".html")) {
                contentType = "text/html";
            } else if (FileDriver::ends_with(filename, ".css")) {
                contentType = "text/css";
            } else if (FileDriver::ends_with(filename, ".js")) {
                contentType = "application/javascript";
            }
            crow::response res;
            res.code = 200;
            res.set_header("Content-Type", contentType);
            res.write(fileContent);
            return res;
        } else {
            return crow::response(404, "File not found");
        }
    });

    CROW_ROUTE(api, "/shutdown")
    ([]() {
        auto response = crow::response(200, "Service shutting down");
        shutdown_flag.store(true);  // Set shutdown flag to true when /shutdown is hit
        return response; });

    CROW_ROUTE(api, "/startup")
    ([]() {
        sAudioDriver.start();
        sMidiDriver.start();
        sPlayerEngine.midiEnable(&sMidiDriver);
        return crow::response(200, "Services started");
    });

    // Endpoint to start audio generation
    CROW_ROUTE(api, "/device/audio/doStart")
    ([]() {
        if (sAudioDriver.start()) {
            return crow::response(200, "Audio started successfully");
        } else {
            return crow::response(500, "Failed to start audio");
        } });

    // Endpoint to stop audio generation
    CROW_ROUTE(api, "/device/audio/doStop")
    ([]() {
        sAudioDriver.stop();
        return crow::response(200, "Audio stopped successfully"); });

    CROW_ROUTE(api, "/device/midi/doStart")
    ([]() {
        sMidiDriver.start();
        //also connect it to player engine..
        sPlayerEngine.midiEnable(&sMidiDriver);
        return crow::response(200, "Midi started successfully"); });

    CROW_ROUTE(api, "/device/midi/doStop")
    ([]() {
        sPlayerEngine.midiDisable();
        sMidiDriver.stop();
        return crow::response(200, "Midi stopped successfully"); });

    CROW_ROUTE(api, "/test/pe/ping")
    ([]() {
        sPlayerEngine.ping();
        return crow::response(200, "Ping sent from PlayerEngine"); });

    CROW_ROUTE(api, "/test/pe/rackSetup")
    ([]() {
        sPlayerEngine.testRackSetup();
        return crow::response(200, "Test synth setup");
    });

    CROW_ROUTE(api, "/rack/getFakeSynthParams")
    ([]() {
        std::string json = sPlayerEngine.getSynthParams(0);
        if (json.empty()) {
            return crow::response(500, "Failed to retrieve synth parameters");
        }

        // Return the JSON response with a 200 OK status
        return crow::response(200, json);
    });

    api.route_dynamic("/setRackUnitParam")([](const crow::request &req) {
        auto rack_str = req.url_params.get("rack");
        auto unit = req.url_params.get("unit");
        auto name = req.url_params.get("name");
        // to speed up parsing, lets accept 0-127 as the range.
        auto value_str = req.url_params.get("value");

        std::cout << "value_str:" << value_str << std::endl;
        // Initialize default values
        int rack = -1; // invalid as default
        int value = 64;

        // Convert rack to integer with fallback
        if (rack_str) {
            try {
                rack = std::stoi(rack_str);
            } catch (...) { // Catch all exceptions
                rack = -1;  // Fallback value
            }
        }

        if (value_str) {
            try {
                value = std::stoi(value_str);
            } catch (...) { // Catch all exceptions
                value = 0;  // Fallback value
            }
        }
        std::cout << "value:" << value << std::endl;

        std::string name_str(name ? name : "");
        Message msg{rack, "synth", name_str.c_str(), value};
        if (sMessageReciever.push(msg)) {
            // Return success message
            return crow::response(200, "Pushed message to the rack");
        } else {
            return crow::response(400, "You're too fast. All your fault.");
        }
    });

    // api.port(18080).run();
    //  when here all closed the right way..
    // std::cout << "Graceful quit right?" << std::endl;
    // return 0;
    /* Threaded CROW - probably not needed */

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
