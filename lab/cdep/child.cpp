#include "child.h"
#include "parent.h"

// Forward declaration of Parent class to avoid circular dependency
// class Parent;

// Child::Child(Parent *parent) : parent(parent) {} // Constructor with Parent pointer
Child::Child(Parent &parent) : parent(parent) {} // Constructor with Parent pointer

float Child::myComplicatedFormula() {
    // hm.. how can i call log() in parent without causing circular dependency?
    /*    if (parent) {
            parent->log("Child is calling Parent's log()!");
        }
        */
    parent.log("Child calling by ref");
    return 1.57f;
}
