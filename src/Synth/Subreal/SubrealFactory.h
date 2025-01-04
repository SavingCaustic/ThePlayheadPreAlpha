#pragma once
#include "Synth/Subreal/SubrealModel.h"
#include "core/audio/envelope/ADSFR.h"
#include "core/audio/osc/LUT.h"
#include "core/constructor/Queue.h"
#include <array>
#include <cmath>
#include <core/utils/FNV.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Synth::Subreal {
class Factory {
  public:
    static void prepareSetting(std::string key, std::string value, int rackID, Constructor::Queue &constructorQueue) {
        // ok so this bascially pushes the record onto the injector queue..
        std::cout << key << std::endl;
        uint32_t keyFNV = Utils::Hash::fnv1a(key);
        switch (keyFNV) {
        case Utils::Hash::fnv1a_hash("lut1_overtones"): {
            createLUT(key, value, rackID, constructorQueue);
            break;
        }
            // push the lut to the queue for object injects. We need to know rack id.
        case Utils::Hash::fnv1a_hash("lut2_overtones"): {
            createLUT(key, value, rackID, constructorQueue);
            break;
        }
        }
    }

    static void createLUT(std::string &key, std::string value, int rackID, Constructor::Queue &constructorQueue) {
        // a 64k LUT to avoid alias on lower frequencies without interpolation.
        std::cout << "creating lut1 now.." << std::endl;
        audio::osc::LUT *lutTmp = new audio::osc::LUT();
        buildLUT(lutTmp, value);
        // Prepend "unit." to the key
        std::string prefixedKey = "synth." + key;

        // Push the LUT into the Constructor Queue
        if (!constructorQueue.push(lutTmp, sizeof(audio::osc::LUT), false, prefixedKey.c_str(), rackID)) {
            std::cerr << "Failed to push LUT into Constructor Queue. Queue is full!" << std::endl;
            delete lutTmp; // Clean up if the push fails
        }

        // Clear the local pointer
        lutTmp = nullptr;
    }

    static void buildLUT(audio::osc::LUT *lut, const std::string val) {
        std::vector<float> values;
        std::istringstream stream(val);
        std::string token;
        int h = 1;
        while (std::getline(stream, token, ',')) {
            lut->applySine(h, std::stof(token));
            h++;
        }
        lut->normalize();
    }

    static void reset(int rackID, Constructor::Queue &constructorQueue) {
        // set some default values. maybe move to json later..
        // prepareSetting("lut1_overtones", "0.5,0.3,0.3,0.2,0.2,0.1", rackID, constructorQueue);
        // prepareSetting("lut2_overtones", "0.5,0.0,0.1", rackID, constructorQueue);
    }
};

} // namespace Synth::Subreal