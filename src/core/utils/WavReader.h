#pragma once
#include <cstdio>
#include <cstring> // For memcpy
#include <string>
#include <vector>

namespace Utils {

struct WavHeader {
    char riff_tag[4];
    int riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    int fmt_length;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char data_tag[4];
    int data_length;
};

class WavReader {
  public:
    WavReader() {}
    ~WavReader() { close(); }

    // Open a file and validate its header
    bool open(const std::string &filename) {
        file = std::fopen(filename.c_str(), "rb");
        if (!file) {
            return false; // File open failed
        }
        if (!readHeader()) {
            close();
            return false; // Invalid WAV file
        }
        return true;
    }

    // Close the file if open
    void close() {
        if (file) {
            std::fclose(file);
            file = nullptr;
        }
    }

    // Retrieve file info
    bool getFileInfo(WavHeader &headerOut) const {
        if (!file)
            return false;
        headerOut = header; // Copy header to the output parameter
        return true;
    }

    // Read the entire WAV file as float
    bool returnWavAsFloat(std::vector<float> &output) {
        if (!file)
            return false;

        // Calculate sample count and preallocate memory
        int sampleCount = header.data_length / header.block_align;
        output.reserve(sampleCount * header.num_channels);

        // Buffer to hold PCM data temporarily
        std::vector<int16_t> buffer(sampleCount * header.num_channels);
        std::fread(buffer.data(), sizeof(int16_t), buffer.size(), file);

        // Convert PCM to float
        output.resize(buffer.size()); // Now set the size to match the reserved memory
        for (size_t i = 0; i < buffer.size(); ++i) {
            output[i] = buffer[i] / 32768.0f; // Normalize to [-1.0, 1.0]
        }

        return true;
    }

    // Read a specific number of samples
    bool returnWavPartAsFloat(std::vector<float> &output, uint16_t samples) {
        if (!file)
            return false;

        // Allocate buffer for requested PCM data
        output.resize(samples * header.num_channels);
        std::vector<int16_t> buffer(samples * header.num_channels);

        // Read from the file
        size_t readCount = std::fread(buffer.data(), sizeof(int16_t), buffer.size(), file);
        if (readCount < buffer.size()) {
            output.resize(readCount); // Trim the output vector
        }

        // Convert PCM to float
        for (size_t i = 0; i < readCount; ++i) {
            output[i] = buffer[i] / 32768.0f; // Normalize to [-1.0, 1.0]
        }

        return true;
    }

  private:
    FILE *file;
    WavHeader header;

    // Read and validate the WAV header
    bool readHeader() {
        if (!file)
            return false;

        if (std::fread(&header, sizeof(WavHeader), 1, file) != 1) {
            return false; // Failed to read header
        }

        // Validate key fields
        if (std::memcmp(header.riff_tag, "RIFF", 4) != 0 ||
            std::memcmp(header.wave_tag, "WAVE", 4) != 0 ||
            std::memcmp(header.fmt_tag, "fmt ", 4) != 0) {
            return false; // Not a valid WAV file
        }

        return true;
    }
};
} // namespace Utils
