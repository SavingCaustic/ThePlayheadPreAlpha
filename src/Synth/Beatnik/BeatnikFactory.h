#pragma once
#include "./BeatnikModel.h"
#include "core/audio/sample/wav.h"
#include "core/factory/constructor/Queue.h"
#include <array>
#include <cmath>
#include <core/utils/FNV.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Synth::Beatnik {
class Factory {
  public:
    static void prepareSetting(std::string key, std::string value, int rackID, Constructor::Queue &constructorQueue) {
        // ok so this bascially pushes the record onto the injector queue..
        std::cout << key << std::endl;
        uint32_t keyFNV = Utils::Hash::fnv1a(key);
        switch (keyFNV) {
        case Utils::Hash::fnv1a_hash("ch0_sample"): {
            loadSample(key, value, rackID, constructorQueue);
            break;
        }
        }
    }

    static void loadSample(const std::string &key, const std::string value, const int rackID, const Constructor::Queue &constructorQueue) {
        // check if file exists, if it's stereo and load it into a object, and pass it to the constructor
        std::cout << "loading sample now.." << std::endl;
        uint32_t sampleSize = 0;
        audio::sample::WavSample *wavTmp = new audio::sample::WavSample(value, sampleSize);
        std::string prefixedKey = "synth." + key;

        // Push the LUT into the Constructor Queue
        if (!constructorQueue.push(wavTmp, sampleSize, false, prefixedKey.c_str(), rackID)) {
            std::cerr << "Failed to push sample into Constructor Queue. Queue is full!" << std::endl;
            delete wavTmp; // Clean up if the push fails
        }

        // Clear the local pointer
        wavTmp = nullptr;
    }

    static void reset(int rackID, Constructor::Queue &constructorQueue) {
        // set some default values. maybe move to json later..
        // prepareSetting("lut1_overtones", "0.5,0.3,0.3,0.2,0.2,0.1", rackID, constructorQueue);
        // prepareSetting("lut2_overtones", "0.5,0.0,0.1", rackID, constructorQueue);
    }
};

} // namespace Synth::Beatnik