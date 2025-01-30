#pragma once

// buffer-len is given in stereo.
class RackEmitter {
  public:
    RackEmitter(float *audioBufferLeft, float *audioBufferRight, int bufferLen)
        : bufferPtrLeft(audioBufferLeft), bufferPtrRight(audioBufferRight), bufferLen(bufferLen) {}

    bool process(bool isStereo) {
        //$this->processEQ($audioBuffer, $isStereo);
        // a pan may convert a mono to stereo in last step..
        isStereo = this->processPanAndFader(isStereo);
        return isStereo;
    }

    bool processPanAndFader(bool isStereo) {
        // if mono, copy left to right.
        float testGain = 1.0f;
        for (int i = 0; i < bufferLen; i++) {
            bufferPtrLeft[i] = bufferPtrLeft[i] * testGain;
        }
        if (isStereo) {
            for (int i = 0; i < bufferLen; i++) {
                bufferPtrRight[i] = bufferPtrRight[i] * testGain;
            }
        } else {
            for (int i = 0; i < bufferLen; i++) {
                bufferPtrRight[i] = bufferPtrLeft[i] * testGain;
            }
        }
        return true;
    }

  private:
    float *bufferPtrLeft, *bufferPtrRight;
    int bufferLen;
};