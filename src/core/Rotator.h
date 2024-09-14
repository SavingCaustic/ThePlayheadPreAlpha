#pragma once
#include "../constants.h"

class Rotator {
  public:
    Rotator();
    ~Rotator();

    void reset();
    void setTempo(int bpm, bool dotted = false);
    bool frameTurn();
    float pulse; // Float 0 - 96 (192PPQN)

  private:
    void setPulsesPerFrame(float eps);

    float pulsesPerFrame; // Float
};