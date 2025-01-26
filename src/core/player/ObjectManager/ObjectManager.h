#pragma once
#include "constants.h"
#include "core/destructor/Queue.h"
#include "core/factory/constructor/Queue.h"
#include "core/player/Rack/Rack.h"
#include "core/utils/FNV.h"
#include <array>
#include <cmath>
#include <iostream>
#include <string>

class ObjectManager {
  public:
    explicit ObjectManager(Rack (&racks)[TPH_RACK_COUNT]);

    void process();

    bool destroySynth(int rackID);

    bool destroyEffect(int rackID, int effectSlot);

    bool destroyEventor(int rackID, int effectSlot);

    Rack (&racks)[TPH_RACK_COUNT]; // Reference to the racks array
    Constructor::Queue *constructorQueue = nullptr;
    Destructor::Queue *destructorQueue = nullptr;
};
