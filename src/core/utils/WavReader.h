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
    WavReader() : file(nullptr) {}
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

    // Return WAV file as float array
    bool returnWavAsFloat(float *output, uint32_t totalSamples) {
        if (!file || totalSamples == 0) {
            return false;
        }

        if (header.bits_per_sample == 16) {
            // Handle 16-bit PCM
            std::vector<int16_t> buffer(totalSamples);
            size_t readCount = std::fread(buffer.data(), sizeof(int16_t), buffer.size(), file);

            if (readCount != totalSamples) {
                return false; // Ensure the correct number of samples is read
            }

            // Convert PCM to float
            for (size_t i = 0; i < totalSamples; ++i) {
                output[i] = buffer[i] / 32768.0f; // Normalize to [-1.0, 1.0]
            }
        } else if (header.bits_per_sample == 24) {
            // Handle 24-bit PCM
            std::vector<uint8_t> buffer(totalSamples * 3); // 3 bytes per sample
            size_t readCount = std::fread(buffer.data(), 3, totalSamples, file);

            if (readCount != totalSamples) {
                return false; // Ensure the correct number of samples is read
            }

            // Convert PCM to float
            for (size_t i = 0; i < totalSamples; ++i) {
                int32_t sample = (buffer[i * 3 + 2] << 16) | (buffer[i * 3 + 1] << 8) | buffer[i * 3];
                // Sign-extend 24-bit to 32-bit
                if (sample & 0x800000) {
                    sample |= ~0xFFFFFF;
                }
                output[i] = sample / 8388608.0f; // Normalize to [-1.0, 1.0]
            }
        } else {
            return false; // Unsupported format
        }

        return true;
    }

    // Read a specific number of samples
    bool returnWavPartAsFloat(std::vector<float> &output, uint16_t samples) {
        if (!file)
            return false;

        output.resize(samples * header.num_channels);

        if (header.bits_per_sample == 16) {
            // Handle 16-bit PCM
            std::vector<int16_t> buffer(samples * header.num_channels);
            size_t readCount = std::fread(buffer.data(), sizeof(int16_t), buffer.size(), file);

            if (readCount < buffer.size()) {
                output.resize(readCount); // Trim the output vector
            }

            // Convert PCM to float
            for (size_t i = 0; i < readCount; ++i) {
                output[i] = buffer[i] / 32768.0f; // Normalize to [-1.0, 1.0]
            }
        } else if (header.bits_per_sample == 24) {
            // Handle 24-bit PCM
            std::vector<uint8_t> buffer(samples * header.num_channels * 3); // 3 bytes per sample
            size_t readCount = std::fread(buffer.data(), 3, samples * header.num_channels, file);

            if (readCount < samples * header.num_channels) {
                output.resize(readCount / 3); // Trim the output vector
            }

            // Convert PCM to float
            for (size_t i = 0; i < readCount / 3; ++i) {
                int32_t sample = (buffer[i * 3 + 2] << 16) | (buffer[i * 3 + 1] << 8) | buffer[i * 3];
                // Sign-extend 24-bit to 32-bit
                if (sample & 0x800000) {
                    sample |= ~0xFFFFFF;
                }
                output[i] = sample / 8388608.0f; // Normalize to [-1.0, 1.0]
            }
        } else {
            return false; // Unsupported format
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
