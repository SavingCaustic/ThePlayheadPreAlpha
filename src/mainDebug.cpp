#include "Synth/DummySin/DummySinModel.h"
#include "Synth/SynthInterface.h"
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
    float audioBuffer[64];
    const std::size_t bufferSize = 64;
    Synth::DummySin::Model mySynth(audioBuffer, bufferSize);
    Utils::WavWriter writer("echo.wav", 48000, 64);

    // play some notes listening for cracks..
    for (int t = 0; t < 50; t++) {
        u_int8_t note = 40 + ((t * 5) % 26);
        mySynth.parseMidi(0x90, note, 0x80);
        for (int i = 0; i < 65; i++) {
            mySynth.renderNextBlock();
            writer.write(audioBuffer);
        }
        mySynth.parseMidi(0x80, note, 0x00);
        for (int i = 0; i < 65; i++) {
            mySynth.renderNextBlock();
            writer.write(audioBuffer);
        }
    }
    writer.close();

    return 0;
}