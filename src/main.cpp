#include "Effect/EffectBase.h"
#include "Effect/EffectFactory.h"
#include "Synth/SynthBase.h"
#include "Synth/SynthFactory.h"
#include "constants.h"
#include "core/api/endpointsCrow.h"
#include "core/api/rpcParser.h"
#include "core/audio/AudioMath.h"
#include "core/destructor/Worker.h"
#include "core/hallways/FactoryHallway.h"
#include "core/logger/AudioLoggerQueue.h"
#include "core/logger/LoggerHandler.h"
#include "core/logger/LoggerQueue.h"
#include "core/messages/MessageInQueue.h"
#include "core/messages/MessageOutQueue.h"
#include "core/messages/MessageOutReader.h"
#include "core/parameters/ProjectSettingsManager.h"
#include "core/parameters/ServerSettingsManager.h"
#include "core/player/PlayerEngine.h"
#include "core/player/Rack/Rack.h"
#include "core/runner/StudioRunner.h"
#include "crow.h"
#include "drivers/AudioDriver.h"
#include "drivers/AudioManager.h"
#include "drivers/FileDriver.h"
#include "drivers/MidiManager.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ### # # ###   ##  #    #  # # # # ###  #  ##   *
 *   #  ### ##    ### #   ###  #  ### ##  ### # #  *
 *   #  # # ###   #   ### # #  #  # # ### # # ##   *
 * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#define DEBUG_MODE 1

// Define the global shutdown flag. Set by endpoint shutdown and signal_handler below
std::atomic<bool> shutdown_flag(false);

// Custom signal handler. Working well.
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Caught SIGINT (Ctrl+C), setting shutdown flag..." << std::endl;
        shutdown_flag = true;
    }
}

// Global objects. Could possibly be hierarchial but could lead to poor testing environments.
PlayerEngine sPlayerEngine;
MessageInQueue sMessageInQueue(8);
MessageOutQueue sMessageOutQueue;
MessageOutReader sMessageOutReader(sMessageOutQueue, nullptr); // Initialize without connection

// for project top-settings..
ProjectSettingsManager sProjectSettingsManager;

Constructor::Queue sConstructorQueue;
// with factories being stateless, it's API and PlayerEngine that needs to know sConstructorQueue
Storage::DataStore sDataStore;

RPCParser rpcParser;
// data and serialization methods kept here:
FactoryHallway factoryHallway;

AudioDriver sAudioDriver;
AudioManager sAudioManager(sAudioDriver, sPlayerEngine);
MidiManager sMidiManager;

// Add global ErrorHandler
AudioLoggerQueue sAudioLoggerQueue;                              // Error buffer used by the audio engine
LoggerQueue sLoggerQueue;                                        // Main error buffer for logging
LoggerHandler sLoggerHandler(&sAudioLoggerQueue, &sLoggerQueue); // Error handler with threads

// DestructorBuffer
Destructor::Queue sDestructorQueue;
Destructor::Worker sDestructorWorker(sDestructorQueue);

// StudioRunner - performing various low-prio task (micro-scheduler)
StudioRunner sStudioRunner(sMidiManager, sAudioManager);

// Entry point of the program
int main() {
    // setup factory hallway
    factoryHallway.constructorQueueMount(sConstructorQueue);
    factoryHallway.loggerMount(sLoggerQueue);
    factoryHallway.datastoreMount(sDataStore);
    factoryHallway.projectSettingsMount(sProjectSettingsManager);
    // Create the object
    crow::SimpleApp api;
    // Remove default signal handler
    api.signal_clear();
    // Add our custom handler
    signal(SIGINT, signal_handler);
    //
    std::unordered_map<std::string, std::string> deviceSettings;
    ServerSettingsManager::jsonRead(deviceSettings, "server.json");
    // initialize
    AudioMath::initialize();
    // Kickstart the StudioRunner (low-priority job scheduler)
    sStudioRunner.reset();

    // meh - almost factory here..
    sPlayerEngine.ccManager.updateMidiSettings(deviceSettings["scroller_cc"], deviceSettings["subscroller_cc"], deviceSettings["scroller_dials"]);
    // postpone this so we wait with the audio-thread..
    // sStudioRunner.start();

    // this could be and endpoint..
    //  now what playerEngine is initiated, setup the callback.
    unsigned long framesPerBuffer = static_cast<unsigned long>(std::stoi(deviceSettings["buffer_size"]));

    //
    sPlayerEngine.bindMessageInQueue(sMessageInQueue);
    sPlayerEngine.bindMessageOutQueue(sMessageOutQueue);
    sPlayerEngine.bindLoggerQueue(sAudioLoggerQueue);
    sPlayerEngine.bindMidiManager(sMidiManager);
    sPlayerEngine.bindConstructorQueue(sConstructorQueue);
    sPlayerEngine.bindDestructorQueue(sDestructorQueue);
    sPlayerEngine.bindProjectSettingsManager(sProjectSettingsManager);

    sLoggerHandler.start();
    sDestructorWorker.start();
    sPlayerEngine.initializeRacks();

    // Start the audio driver
    sAudioManager.mountPreferedOrDefault(deviceSettings["audio_device"]);
    sStudioRunner.start();

    //
    crowSetupEndpoints(api, sMessageInQueue, sMessageOutQueue, sMessageOutReader, rpcParser);
    int httpPort = std::stoi(deviceSettings["http_port"]);
    std::thread server_thread([&api, httpPort]() { api.port(httpPort).run(); });
    while (!shutdown_flag.load()) {
        if (false & DEBUG_MODE) {
            std::cout << "housekeeping.. " << std::endl;
            // should be errorBuffer sErrorHandler.addError(200, "testing error");
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    std::cout << "beginning shutdown.." << std::endl;
    sMessageOutReader.stop();
    std::cout << "stopped web-socket feeder" << std::endl;
    //
    std::cout << "stopping destructor...";
    sDestructorWorker.start();
    std::cout << "done.\n";
    //
    std::cout << "Stopping the server..." << std::endl;
    api.stop();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (server_thread.joinable()) {
        server_thread.join();
    }
    sLoggerHandler.stopThreads();

    sStudioRunner.stop();
    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
