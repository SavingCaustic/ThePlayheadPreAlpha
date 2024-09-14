#pragma once
#include <string>
#include <vector>

class SynthInterface {
  public:
    virtual ~SynthInterface() = default;
    virtual void reset() = 0;
    virtual void setParam(const std::string &name, float val) = 0;
    virtual void parseMidi(int cmd, int param1, int param2) = 0;
    virtual bool renderNextBlock() = 0;
};
