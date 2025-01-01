#pragma once
#include <fstream>
#include <string>

// having second thoughts regarding the statelessness here.
// in a WEBDAV or SSH-scenario, this indeed would have a state.
// worst case would be a separate thread..
// But well, having WEBDAV or SSH like a transparent medium wouldn't work - too much latency.
// project-factory could download to device.

class FileDriver {
  public:
    // Static methods for file I/O
    // Barely a driver since it's stateless..
    static std::string secure(const std::string &filename);
    static bool assetFileExists(const std::string &filename);
    static std::string assetFileRead(const std::string &filename);
    static bool userFileExists(const std::string &filename);
    static std::string userFileRead(const std::string &filename);
    static bool userFileWrite(const std::string &filename, const std::string &content);
    static std::string load_file_content(const std::string &filepath);
    static bool ends_with(const std::string &str, const std::string &suffix);
};
