#include "WavWriter.h"
#include <algorithm>
#include <cmath>
#include <cstring>
namespace Utils {
WavWriter::WavWriter() {};

bool WavWriter::open(const std::string &filename, int sample_rate, int num_channels) {
    file = 0;
    samples_written = 0;
    int bits_per_sample = 16;

    std::strncpy(header.riff_tag, "RIFF", 4);
    std::strncpy(header.wave_tag, "WAVE", 4);
    std::strncpy(header.fmt_tag, "fmt ", 4);
    std::strncpy(header.data_tag, "data", 4);

    header.riff_length = 0;
    header.fmt_length = 16;
    header.audio_format = 1;
    header.num_channels = num_channels; // Use the parameter for channels
    header.sample_rate = sample_rate;
    header.bits_per_sample = bits_per_sample;

    // Calculate byte_rate and block_align correctly
    header.byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
    header.block_align = num_channels * (bits_per_sample / 8);
    header.data_length = 0;

    file = std::fopen(filename.c_str(), "wb+");
    if (file) {
        writeHeader();
    }
    return true;
}

WavWriter::~WavWriter() {
    close();
}

bool WavWriter::isOpen() const {
    return file != nullptr;
}

void WavWriter::write(const float *dataLeft, const float *dataRight, std::size_t size) {
    if (file) {
        std::size_t i = 0;
        while (i < size) {
            std::size_t blockSize = std::min(size - i, std::size_t(1024)); // max 1024 samples per loop

            // Process left and right channel data and clamp to short
            for (std::size_t j = 0; j < blockSize; ++j) {
                // Convert and store left channel sample
                short_data[j * 2] = static_cast<short>(std::clamp(dataLeft[i + j], -1.0f, 1.0f) * 32767);
                // Convert and store right channel sample
                short_data[j * 2 + 1] = static_cast<short>(std::clamp(dataRight[i + j], -1.0f, 1.0f) * 32767);
            }

            // Write the interleaved data to the file
            std::fwrite(short_data, sizeof(short), blockSize * 2, file); // 2 because we have left and right channels
            i += blockSize;
        }
        samples_written += size;
    }
}

void WavWriter::writeMono(const float *data, std::size_t size) {
    if (file) {
        std::size_t i = 0;
        while (i < size) {
            std::size_t blockSize = std::min(size - i, std::size_t(1024)); // max 1024 samples per loop

            // Process left and right channel data and clamp to short
            for (std::size_t j = 0; j < blockSize; ++j) {
                // Convert and store mono channel sample
                short_data[j] = static_cast<short>(std::clamp(data[i + j], -1.0f, 1.0f) * 32767);
            }

            // Write the interleaved data to the file
            std::fwrite(short_data, sizeof(short), blockSize, file); // 2 because we have left and right channels
            i += blockSize;
        }
        samples_written += size;
    }
}

void WavWriter::close() {
    if (file) {
        updateLengths();
        std::fclose(file);
        file = nullptr;
    }
}

void WavWriter::writeHeader() {
    if (file) {
        std::fwrite(&header, sizeof(header), 1, file);
        std::fflush(file);
    }
}

void WavWriter::updateLengths() {
    if (!file)
        return;

    int file_length = std::ftell(file);
    int data_length = samples_written * sizeof(short);
    header.data_length = data_length;
    header.riff_length = file_length - 8;

    std::fseek(file, sizeof(Header) - sizeof(int32_t), SEEK_SET);
    std::fwrite(&header.data_length, sizeof(header.data_length), 1, file);

    std::fseek(file, 4, SEEK_SET);
    std::fwrite(&header.riff_length, sizeof(header.riff_length), 1, file);
}
} // namespace Utils