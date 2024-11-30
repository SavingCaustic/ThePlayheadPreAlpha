#pragma once

namespace Admin {
enum Target {
    RACK,
    SONG,
    PROJECT
};

// this defines records in the queue.

struct AdminRec {
    Target target;
    int tagetID;
    char subTargetComponent[20];
    float f1 = 0.0f, f2 = 0.0f; // Generic float parameters
    char s1[255];
    char s2[255];
};
} // namespace Admin
