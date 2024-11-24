#include <hash_map>
#include <iostream>

class CCRouter {
  public:
    // Singleton access
    static CCRouter &getInstance() {
        static CCRouter instance; // Guaranteed to be destroyed, instantiated on first use
        return instance;
    }

    // Load CC mappings (called once, during initialization)
    void loadMappings(const std::unordered_map<int, std::string> &mappings) {
        ccMappings = mappings;
    }

    // Route CC value to the appropriate parameter (read-only operation)
    void routeCC(int ccId, int value) const {
        if (ccMappings.find(ccId) != ccMappings.end()) {
            std::cout << "Routing CC " << ccId << " to parameter: "
                      << ccMappings.at(ccId) << " with value: " << value << std::endl;
        } else {
            std::cout << "No mapping found for CC " << ccId << std::endl;
        }
    }

  private:
    CCRouter() = default;

    // Private copy constructor and assignment operator to prevent copying
    CCRouter(const CCRouter &) = delete;
    CCRouter &operator=(const CCRouter &) = delete;

    // Internal data
    std::unordered_map<int, std::string> ccMappings;
};

// Synth class using the Singleton CC-router
class Synth {
  public:
    Synth() {
        // Example usage: Load mappings into the shared CC-router
        std::unordered_map<int, std::string> mappings = {
            {74, "Filter Cutoff"},
            {71, "Resonance"},
            {5, "Envelope Amount"}};
        CCRouter::getInstance().loadMappings(mappings);
    }

    void processCC(int ccId, int value) {
        // Route the CC value using the shared CCRouter instance
        CCRouter::getInstance().routeCC(ccId, value);
    }
};
