// #include "Synth/Monolith/MonolithModel.h"
#include "Synth/Subreal/SubrealModel.h"
#include "Synth/SynthInterface.h"
#include "core/audio/AudioMath.h"
#include "core/parameters/SettingsManager.h"
#include "core/player/Rack.h"
#include "core/utils/WavWriter.h"
#include <atomic>
#include <iostream>
#include <signal.h>

#define DEBUG_MODE 1
// Define the global shutdown flag. Set by endpoint shutdown and signal_handler below

// For step debugging to properly work, build_debug has to be created and cmake invoked using
//  cmake .. -DCMAKE_BUILD_TYPE=Debug
//  verify binary contains debug-info using "file MyExe"

std::atomic<bool> shutdown_flag(false);

// Custom signal handler. Working well.
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Caught SIGINT (Ctrl+C), setting shutdown flag..." << std::endl;
        shutdown_flag = true;
    }
}

int main() {
    std::unordered_map<std::string, std::string> deviceSettings;
    SettingsManager::loadJsonToSettings("device.json", true, deviceSettings);

    AudioMath::initialize();
    Rack myRack = Rack();
    myRack.setSynthFromStr("Subreal");

    int blockSize = 128; // rack-render-size always 64 samples, so in stereo = 128
    Utils::WavWriter writer;
    writer.open("echo.wav", 48000, 2);
    for (int t = 0; t < 20; t++) {
        if (t % 10 == 0) {
            float v = t;
            v = t / 50.0f;
            // mySynth.pushStrParam("osc1_wf", v);
            // mySynth.handleMidiCC(74, 0.8);
        }
        u_int8_t note = 40 + ((t * 5) % 26);
        // note = 60;
        myRack.parseMidi(0x90, note, 0x70);
        // myRack.parseMidi(0x90, note + 3, 0x70);
        // myRack.parseMidi(0x90, note + 7, 0x70);
        for (int i = 0; i < 65; i++) {
            myRack.render(blockSize);
            writer.write(myRack.audioBuffer.data(), myRack.audioBuffer.size());
        }
        // myRack.parseMidi(0x80, note + 7, 0x00);
        // myRack.parseMidi(0x80, note + 3, 0x00);
        myRack.parseMidi(0x80, note, 0x00);
        for (int i = 0; i < 50; i++) {
            myRack.render(blockSize);
            writer.write(myRack.audioBuffer.data(), myRack.audioBuffer.size());
        }
    }
    writer.close();

    return 0;
}
