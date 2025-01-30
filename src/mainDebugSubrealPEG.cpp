// #include "Synth/Monolith/MonolithModel.h"
#include "Synth/Subreal/SubrealFactory.h"
#include "Synth/Subreal/SubrealModel.h"
#include "Synth/SynthFactory.h"
#include "Synth/SynthInterface.h"
#include "core/audio/AudioMath.h"
#include "core/parameters/SettingsManager.h"
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

int debugSubrealPEG() {
    // instead of mocking the whole contructor-queue, we're working directly with a rack..
    std::unordered_map<std::string, std::string> deviceSettings;
    SettingsManager::loadJsonToSettings("device.json", true, deviceSettings);

    AudioMath::initialize();
    Rack myRack = Rack();
    // can i setup Subreal here? And pass it to rack?
    Synth::Subreal::Model *mySubreal = new Synth::Subreal::Model();

    audio::osc::LUT *lutTmp = new audio::osc::LUT();
    Synth::Subreal::Factory::buildLUT(lutTmp, "0.8,0.1,0.1");
    mySubreal->lut1 = lutTmp;
    mySubreal->updateVoiceLUT(*lutTmp, 1);
    lutTmp = nullptr;
    //
    audio::osc::LUT *lutTmp2 = new audio::osc::LUT();
    Synth::Subreal::Factory::buildLUT(lutTmp2, "0.9,0.0,0.0");
    mySubreal->lut2 = lutTmp2;
    mySubreal->updateVoiceLUT(*lutTmp2, 2);
    lutTmp2 = nullptr;
    //
    myRack.setSynth(mySubreal);

    // ok, render some..
    int blockSize = 128; // rack-render-size always 64 samples, so in stereo = 128
    Utils::WavWriter writer;
    writer.open("echo.wav", 48000, 2);
    myRack.parseMidi(0x90, 64, 100);
    for (int i = 0; i < 900; i++) {
        myRack.render(blockSize);
        writer.write(myRack.audioBufferLeft.data(), myRack.audioBufferRight.data(), TPH_RACK_RENDER_SIZE);
        if (i == 800) {
            lutTmp2 = nullptr;
        }
    }
    myRack.parseMidi(0x80, 64, 0);
    for (int i = 0; i < 900; i++) {
        myRack.render(blockSize);
        writer.write(myRack.audioBufferLeft.data(), myRack.audioBufferRight.data(), TPH_RACK_RENDER_SIZE);
    }
    writer.close();

    return 0;
}
