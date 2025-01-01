#pragma once
#include <iostream>>

// used by Synths, Effects and Eventors for secure object deletion (by StudioRunner)

class PlayheadUnit {
  public:
    virtual ~PlayheadUnit() {
        std::cout << "DawUnit destroyed\n";
    }
};
