#include "Synth/Subreal/SubrealFactory.h"
#include "Synth/Subreal/SubrealModel.h"
#include "Synth/SynthFactory.h"
#include "Synth/SynthInterface.h"
#include "core/audio/AudioMath.h"
#include "core/parameters/SettingsManager.h"
#include "core/player/Rack/Rack.h"
#include "core/storage/DataStore.h"
#include "core/utils/WavWriter.h"
#include <atomic>
#include <iostream>
#include <signal.h>

#define DEBUG_MODE 1
// Define the global shutdown flag. Set by endpoint shutdown and signal_handler below

// For step debugging to properly work, build_debug has to be created and cmake invoked using
//  cmake .. -DCMAKE_BUILD_TYPE=Debug
//  verify binary contains debug-info using "file MyExe"

int debugPatchLoad() {
    // instead of mocking the whole contructor-queue, we're working directly with a rack..
    std::unordered_map<std::string, std::string> deviceSettings;
    SettingsManager::loadJsonToSettings("device.json", true, deviceSettings);

    AudioMath::initialize();
    Rack myRack = Rack();
    // can i setup Subreal here? And pass it to rack?
    Synth::Subreal::Model *mySubreal = new Synth::Subreal::Model();
    myRack.setSynth(mySubreal);

    Storage::DataStore dataStore;
    dataStore.loadProject("demo");
    dataStore.loadSynthPatch("Submarino", 0);

    // BAD CODE HERE: SynthFactory::patchLoad(mySubreal, "Submarino");

    // ok, render some..
    int blockSize = 128; // rack-render-size always 64 samples, so in stereo = 128
    Utils::WavWriter writer;
    writer.open("echo.wav", 48000, 2);
    myRack.parseMidi(0x90, 60, 40);
    for (int i = 0; i < 100; i++) {
        myRack.render(blockSize);
        writer.write(myRack.audioBuffer.data(), myRack.audioBuffer.size());
    }
    myRack.parseMidi(0x90, 60, 100);
    for (int i = 0; i < 100; i++) {
        myRack.render(blockSize);
        writer.write(myRack.audioBuffer.data(), myRack.audioBuffer.size());
    }
    myRack.parseMidi(0x90, 61, 100);
    for (int i = 0; i < 6000; i++) {
        myRack.render(blockSize);
        writer.write(myRack.audioBuffer.data(), myRack.audioBuffer.size());
    }
    for (int i = 0; i < 1; i++) {
        myRack.parseMidi(0x80, 60, 0);
    }
    for (int i = 0; i < 100; i++) {
        myRack.render(blockSize);
        writer.write(myRack.audioBuffer.data(), myRack.audioBuffer.size());
    }

    writer.close();

    return 0;
}
