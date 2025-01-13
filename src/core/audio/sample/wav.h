#include "core/utils/WavReader.h"
#include <cstdint> // For int16_t
#include <stdexcept>
#include <string>
#include <vector>

namespace audio::sample {

struct SampleData {
    std::vector<float> data; // Normalized audio samples
    int sampleRate;          // Sample rate
    int numChannels;         // Number of channels
};

void loadSample(SampleData &sample, const std::string &filepath) {
    // Assume WavReader provides header info and PCM data
    Utils::WavReader wavReader;
    if (!wavReader.open(filepath)) {
        // could fall back to read err.wav
        throw std::runtime_error("Failed to open WAV file");
    }

    Utils::WavHeader header;
    if (!wavReader.getFileInfo(header)) {
        throw std::runtime_error("Failed to retrieve WAV header");
    }

    // Calculate the total number of floats
    int totalSamples = header.data_length / header.block_align * header.num_channels;

    // Reserve memory in advance
    sample.data.reserve(totalSamples);

    // Load and convert WAV data
    if (!wavReader.returnWavAsFloat(sample.data)) {
        throw std::runtime_error("Failed to load WAV data");
    }

    sample.sampleRate = header.sample_rate;
    sample.numChannels = header.num_channels;
};
} // namespace audio::sample