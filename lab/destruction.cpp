#include <iostream>
#include <queue>

class DawUnit {
  public:
    virtual ~DawUnit() {
        std::cout << "DawUnit destroyed\n";
    }
};

class Synth : public DawUnit {
  public:
    ~Synth() override {
        std::cout << "Synth destroyed\n";
    }
};

class Effect : public DawUnit {
  public:
    ~Effect() override {
        std::cout << "Effect destroyed\n";
    }
};

class Eventor : public DawUnit {
  public:
    ~Eventor() override {
        std::cout << "Eventor destroyed\n";
    }
};

// Queue for destruction
std::queue<DawUnit *> destructionQueue;

// Book an object for destruction
void bookForDestruction(DawUnit *unit) {
    destructionQueue.push(unit);
}

// Process destruction queue
void processDestructionQueue() {
    while (!destructionQueue.empty()) {
        DawUnit *unit = destructionQueue.front();
        destructionQueue.pop();
        delete unit; // Calls the correct destructor
    }
}

int main() {
    // Create objects
    bookForDestruction(new Synth());
    bookForDestruction(new Effect());
    bookForDestruction(new Eventor());

    // Process the destruction queue
    processDestructionQueue();

    return 0;
}
