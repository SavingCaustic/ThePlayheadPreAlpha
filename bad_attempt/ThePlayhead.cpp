#pragma once
#include "ThePlayhead.h"
#include "core/PlayerEngine.h"
#include "core/errors/ErrorBuffer.h"
#include "core/errors/ErrorHandler.h"
#include "core/messages/MessageOutBuffer.h"
#include "core/messages/MessageOutReader.h"
#include "drivers/AudioDriver.h"
#include "drivers/MidiDriver.h"

// Define the static members
PlayerEngine ThePlayhead::sPlayerEngine;
MessageInBuffer ThePlayhead::sMessageInBuffer(8); // Initialize buffer size
MessageOutBuffer ThePlayhead::sMessageOutBuffer;
MessageOutReader sMessageOutReader(sMessageOutBuffer, nullptr); // Initialize without connection
AudioDriver ThePlayhead::sAudioDriver;
MidiDriver ThePlayhead::sMidiDriver;
AudioErrorBuffer ThePlayhead::sAudioErrorBuffer;
ErrorBuffer ThePlayhead::sErrorBuffer;
ErrorHandler ThePlayhead::sErrorHandler(&ThePlayhead::sAudioErrorBuffer, &ThePlayhead::sErrorBuffer);
// crow::SimpleApp ThePlayhead::api;
ThePlayhead *ThePlayhead::instance = nullptr;

ThePlayhead::ThePlayhead() {
    // No member initialization needed here
}

ThePlayhead::~ThePlayhead() {
    shutdown();
}

void ThePlayhead::initialize() {
    instance = this;
    setSignalHandler();
    loadDeviceSettings();
    initializeAudio();
    bindBuffers();
    startErrorHandler();
    generateAudioLUT();
    /* Temporarily disabling ServerContext to find problems with this file.
        setupServerContext();
        startServer();
        */
}

void ThePlayhead::setSignalHandler() {
    // signal(SIGINT, ThePlayhead::signalHandler);
}

void ThePlayhead::loadDeviceSettings() {
    deviceSettings["buffer_size"] = 128;
    deviceSettings["audio_sr"] = 48000;
    deviceSettings["audio_device"] = 1;
    deviceSettings["midi_device"] = 1;
    deviceSettings["http_port"] = 18080;

    // Override default settings from JSON
    SettingsManager::loadJsonToSettings("device.json", true, deviceSettings);
}

void ThePlayhead::initializeAudio() {
    framesPerBuffer = std::get<int>(deviceSettings["buffer_size"]);
    sAudioDriver.registerCallback([this](float *buffer, unsigned long) {
        sPlayerEngine.renderNextBlock(buffer, framesPerBuffer);
    });
}

void ThePlayhead::bindBuffers() {
    sPlayerEngine.bindMessageInBuffer(sMessageInBuffer);
    sPlayerEngine.bindMessageOutBuffer(sMessageOutBuffer);
    sPlayerEngine.bindErrorBuffer(sAudioErrorBuffer);
}

void ThePlayhead::startErrorHandler() {
    sErrorHandler.start();
}

void ThePlayhead::generateAudioLUT() {
    AudioMath::generateLUT();
}

/* Temporarily disabling ServerContext to find problems with this file.
void ThePlayhead::setupServerContext() {
    ServerContext serverContext(sPlayerEngine, sAudioDriver, sMidiDriver, sMessageInBuffer, sMessageOutBuffer, sMessageOutReader, *this);
    crowSetupEndpoints(api, serverContext);
}

void ThePlayhead::startServer() {
    int httpPort = std::get<int>(deviceSettings["http_port"]);
    server_thread = std::thread([this, httpPort]() { api.port(httpPort).run(); });
}
*/

void ThePlayhead::setShutdownFlag() {
    shutdown_flag = true;
}

void ThePlayhead::run() {
    while (!shutdown_flag.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    shutdown();
}

void ThePlayhead::shutdown() {
    if (!shutdown_flag.load()) {
        shutdown_flag = true;
    }

    std::cout << "Stopping the server..." << std::endl;

    /*
    api.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }
    */

    sErrorHandler.stopThreads();
    std::cout << "Server stopped gracefully." << std::endl;
}