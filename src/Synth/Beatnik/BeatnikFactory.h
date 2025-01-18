#pragma once
#include "./BeatnikModel.h"
#include "core/audio/sample/SimpleSample.h"
#include "core/factory/constructor/Queue.h"
#include "core/utils/WavReader.h"
#include <array>
#include <cmath>
#include <core/utils/FNV.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// yeah on the right track but needs attending once debug-script is working..
namespace Synth::Beatnik {
class Factory {
  public:
    static void prepareSetting(std::string key, std::string value, int rackID, Constructor::Queue &constructorQueue) {
        // ok so this bascially pushes the record onto the injector queue..
        std::cout << key << std::endl;
        uint32_t keyFNV = Utils::Hash::fnv1a(key);
        switch (keyFNV) {
        case Utils::Hash::fnv1a_hash("A_sample"): {
            createSample(key, value, rackID, constructorQueue);
            break;
        }
        }
    }

    static void createSample(std::string &key, std::string value, int rackID, Constructor::Queue &constructorQueue) {
        // check if file exists, if it's stereo and load it into a object, and pass it to the constructor
        std::cout << "loading sample now.." << std::endl;
        uint32_t sampleSize = 0;
        audio::sample::SimpleSample *wavTmp = new audio::sample::SimpleSample(); // value, sampleSize);
        std::string prefixedKey = "synth." + key;

        // Push the LUT into the Constructor Queue
        if (!constructorQueue.push(wavTmp, sampleSize, false, prefixedKey.c_str(), rackID)) {
            std::cerr << "Failed to push sample into Constructor Queue. Queue is full!" << std::endl;
            delete wavTmp; // Clean up if the push fails
        }

        // Clear the local pointer
        wavTmp = nullptr;
    }

    static int buildSample(audio::sample::SimpleSample *sample, const std::string &filename) {
        // local variables - static 100%
        Utils::WavReader reader;
        Utils::WavHeader header;

        if (!sample) {
            std::cerr << "Invalid sample pointer." << std::endl;
            return -1;
        }

        if (!reader.open("assets/Synth/Beatnik/samples/" + filename)) {
            std::cerr << "Failed to open WAV file: " << filename << std::endl;
            return -1;
        }

        if (!reader.getFileInfo(header)) {
            std::cerr << "Failed to read WAV file info" << std::endl;
            return -1;
        }

        // Calculate total samples
        uint32_t totalSamples = header.data_length / header.block_align;

        // Allocate memory for the raw sample data.
        // here we should add some end-silence to avoid constant range check when playing..
        float *sampleData = new float[totalSamples];

        // Load WAV data directly into the allocated memory
        if (!reader.returnWavAsFloat(sampleData, totalSamples)) {
            std::cerr << "Failed to load WAV data" << std::endl;
            delete[] sampleData; // Free memory on failure
            return -1;
        }

        // Mount the sample. ==2 is not default but makes mono = false
        sample->mountSample(sampleData, totalSamples, header.num_channels == 2);
        return 0;
    }
};
} // namespace Synth::Beatnik