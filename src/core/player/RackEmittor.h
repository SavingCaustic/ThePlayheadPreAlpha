#pragma once
// buffer-len is given in stereo.
class RackEmittor {
  public:
    RackEmittor(float *audioBuffer, int bufferLen)
        : bufferPtr(audioBuffer), bufferLen(bufferLen) {}

    bool process(bool isStereo) {
        //$this->processEQ($audioBuffer, $isStereo);
        // a pan may convert a mono to stereo in last step..
        isStereo = this->processPanAndFader(isStereo);
        return isStereo;
    }

    bool processPanAndFader(bool isStereo) {
        // if mono, copy left to right.
        int rightOffset = (isStereo) ? 1 : 0;
        float testGain = 1.0f;
        for (int i = 0; i < bufferLen; i = i + 2) {
            bufferPtr[i] = bufferPtr[i] * testGain;
            bufferPtr[i + 1] = bufferPtr[i + rightOffset] * testGain;
        }
        return true;
    }

  private:
    float *bufferPtr;
    int bufferLen;
};