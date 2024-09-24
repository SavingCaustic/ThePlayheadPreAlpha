#pragma once
#include <fstream>
#include <string>

class FileDriver {
  public:
    // Static methods for file I/O
    // Barely a driver since it's stateless..
    static std::string readAssetFile(const std::string &filename);
    static std::string readUserFile(const std::string &filename);
    static bool writeUserFile(const std::string &filename, const std::string &content);
};
