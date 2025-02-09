#pragma once
#include <cstdio>
#include <string>
#include <vector>

namespace Utils {

class WavWriter {
  public:
    WavWriter();
    ~WavWriter();

    bool open(const std::string &filename, int sample_rate, int num_channels);
    bool isOpen() const;
    void write(const float *dataLeft, const float *dataRight, std::size_t size);
    void writeInterleaved(const float *data, std::size_t size);
    void writeMono(const float *data, std::size_t size);
    // voide writeMono..
    void close();

  private:
    void writeHeader();
    void updateLengths();

    struct Header {
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
    } header;

    FILE *file;
    int samples_written;
    int dataSize;
    short short_data[1024]; // 1024 fixed and max and INTERLEAVED
};
} // namespace Utils
