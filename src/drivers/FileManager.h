#pragma once
#include "FileDriver.h"
#include "constants.h"
#include <iostream>
#include <memory>
#include <string>

class FileManager {
    // this is just food for thought..

    // i need a way to pre-load any samples and other files that are required by the deterministic thread.
    // now their preparations could be done outside deterministic thread.

    // currently, i can't see any other need than cache for samples. JSON-files are unserialized into units
    // in other ways.. And what's left? The webserver delivering static content to client doesn't need this.

    // probably, this class is not needed. The file-system doesn't have "hot-plugging" like midi and audio.
};