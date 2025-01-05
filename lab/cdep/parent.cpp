#include "parent.h"

// Parent::Parent() : child(this) {} // Initialize child with a pointer to parent
Parent::Parent() : child(*this) {} // Initialize child with a pointer to parent

void Parent::test() {
    std::cout << child.myComplicatedFormula() << std::endl;
    log("log");
}

void Parent::log(const std::string &logText) {
    // being called from child
    std::cout << logText << std::endl;
}
