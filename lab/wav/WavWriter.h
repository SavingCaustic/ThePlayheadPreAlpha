#ifndef WAVWRITER_H
#define WAVWRITER_H

#include <cstdio>
#include <string>
#include <vector>

class WavWriter {
  public:
    WavWriter(const std::string &filename, int sample_rate, int dataSize);
    ~WavWriter();

    bool isOpen() const;
    void write(const float *data); // Updated to take a pointer to float array
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
    short short_data[1024]; // 1024 fixed and max.
};

#endif
