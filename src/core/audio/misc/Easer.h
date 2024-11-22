#pragma once
#include <constants.h>
#include <iostream>
// can't see this being used anywhere really..

// maybe this should just be a struct..
// but it's important that voices react to changes of these base settings.
namespace audio::misc {

struct Easer {
    bool active = false;
    float targetValue = 0;
    float currentValue = 0;
    float delta;

    void setTarget(float target);

    inline float getValue() {
        if (this->active) {
            return this->getNewValue();
        } else {
            return currentValue;
        }
    }

    float getNewValue();
};
} // namespace audio::misc
