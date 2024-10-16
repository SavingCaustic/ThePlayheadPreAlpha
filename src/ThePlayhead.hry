#pragma once

#include "core/parameters/SettingsManager.h"
#include <atomic>
#include <condition_variable>
#include <string>
#include <variant>

// Forward declarations
class PlayerEngine;
class MessageInBuffer;
class MessageOutBuffer;
class MessageOutReader;
class AudioDriver;
class MidiDriver;
class AudioErrorBuffer;
class ErrorBuffer;
class ErrorHandler;
class SettingsManager;
class ServerContext;
// namespace crow { class SimpleApp; }

// #include "constants.h"
//  #include "endpointsCrow.h"
// #include <chrono>
// #include <thread>
// #include <unordered_map>

class ThePlayhead {
  public:
    ThePlayhead();
    ~ThePlayhead();

    void initialize();
    void run();
    void setShutdownFlag();
    void shutdown();

  private:
    static PlayerEngine sPlayerEngine;
    static MessageInBuffer sMessageInBuffer;
    static MessageOutBuffer sMessageOutBuffer;
    static MessageOutReader sMessageOutReader;
    static AudioDriver sAudioDriver;
    static MidiDriver sMidiDriver;
    static AudioErrorBuffer sAudioErrorBuffer;
    static ErrorBuffer sErrorBuffer;
    static ErrorHandler sErrorHandler;
    static std::condition_variable cv;

    std::atomic<bool> shutdown_flag;
    std::unordered_map<std::string, VariantType> deviceSettings;
    /*
    std::thread server_thread;
    static crow::SimpleApp api;
    */
    unsigned long framesPerBuffer;

    // Static signal handler
    // static void signalHandler(int signal);

    // Static pointer to the current instance of ThePlayhead
    static ThePlayhead *instance;
};
