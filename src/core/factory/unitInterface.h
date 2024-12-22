#pragma once
#include <string>

class UnitInterface {
    static virual void prepareSetting(std::string key, std::string value, int rackID) = 0;
};