#include "Synth/Beatnik/BeatnikFactory.h"
#include "Synth/Beatnik/BeatnikModel.h"
#include "Synth/SynthFactory.h"
#include "Synth/SynthInterface.h"
#include "core/audio/AudioMath.h"
#include "core/parameters/SettingsManager.h"
#include "core/player/PlayerEngine.h"
#include "core/player/Rack/Rack.h"
#include "core/utils/WavWriter.h"
#include <atomic>
#include <iostream>
#include <signal.h>

#define DEBUG_MODE 1
// Define the global shutdown flag. Set by endpoint shutdown and signal_handler below

// For step debugging to properly work, build_debug has to be created and cmake invoked using
//  cmake .. -DCMAKE_BUILD_TYPE=Debug
//  verify binary contains debug-info using "file MyExe"

int debugPE() {
    std::unordered_map<std::string, std::string> deviceSettings;
    SettingsManager::loadJsonToSettings("device.json", true, deviceSettings);
    AudioMath::initialize();
    // start playerEngine and point myRack to the rack-collection somehow..
    MidiManager sMidiManager;
    PlayerEngine PE;
    PE.bindMidiManager(sMidiManager);
    // PE.hRotator.setTempo(88);
    //
    PE.initializeRacks();
    PE.test = true;
    Rack *myRack = &PE.racks[0];

    Synth::Beatnik::Model *myBeatnik = new Synth::Beatnik::Model();

    audio::sample::SimpleSample *sampleTmp = new audio::sample::SimpleSample();
    Synth::Beatnik::Factory::buildSample(sampleTmp, "lm-2/conga-h.wav");
    myBeatnik->samples[0] = *sampleTmp;
    std::cout << "sample length is: " << sampleTmp->length << std::endl;
    sampleTmp = nullptr;
    //
    sampleTmp = new audio::sample::SimpleSample();
    Synth::Beatnik::Factory::buildSample(sampleTmp, "test/Stereo.wav"); //"lm-2/snare-m.wav");
    myBeatnik->samples[1] = *sampleTmp;
    std::cout << "sample length is: " << sampleTmp->length << std::endl;
    sampleTmp = nullptr;
    //
    myRack->setSynth(myBeatnik);
    //
    Effect::Delay::Model *myDelay = new Effect::Delay::Model();
    myRack->setEffect(myDelay, 1);

    // um. How to test-parse midi in playerEngine? Rack good for now..
    //  ok, render some..
    int blockSize = 128; // rack-render-size always 64 samples, so in stereo = 128
    float stereoBuffer[128];
    Utils::WavWriter writer;
    writer.open("echo.wav", 48000, 2);
    myRack->parseMidi(0x90, 60, 40);
    for (int i = 0; i < 100; i++) { // 100
        PE.renderNextBlock(stereoBuffer, blockSize);
        writer.writeInterleaved(stereoBuffer, blockSize);
    }
    myRack->parseMidi(0x90, 60, 100);
    for (int i = 0; i < 100; i++) {
        PE.renderNextBlock(stereoBuffer, blockSize);
        writer.writeInterleaved(stereoBuffer, blockSize);
    }
    myRack->parseMidi(0x90, 61, 100);
    for (int i = 0; i < 100; i++) {
        PE.renderNextBlock(stereoBuffer, blockSize);
        writer.writeInterleaved(stereoBuffer, blockSize);
    }
    for (int i = 0; i < 25000; i++) {
        PE.renderNextBlock(stereoBuffer, blockSize);
        writer.writeInterleaved(stereoBuffer, blockSize);
    }
    writer.close();

    return 0;
}
