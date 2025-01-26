#include "rpcParser.h"

void RPCParser::parse(const std::string &strClass, const std::string &strMethod, const std::string &strKey,
                      const std::string &strVal, const std::string &rackID, const std::string &unit) {
    // ok. for simplicity.
    uint32_t classFNV = Utils::Hash::fnv1a(strClass);
    uint32_t methodFNV = Utils::Hash::fnv1a(strMethod);
    switch (classFNV) {
    case Utils::Hash::fnv1a_hash("server"):
        std::cout << "parsing verb: " << strMethod << std::endl;
        Factory::Server::parse(methodFNV, "", "");
        break;
    case Utils::Hash::fnv1a_hash("device"):
        // thinking of skipping this. Device could be seen as server setting.
        break;
    case Utils::Hash::fnv1a_hash("project"):
        // might need the constructor queue for master effects (outside racks)
        Factory::Project::parse(strMethod, strKey, strVal); // synth, set, monolith
        break;
    case Utils::Hash::fnv1a_hash("rack"):
        std::cout << "rack here we go.." << std::endl;
        Factory::Rack::parse(strMethod, strKey, strVal, stoi(rackID)); // synth, set, monolith
        break;
    case Utils::Hash::fnv1a_hash("unit"):
        std::cout << "unit here we go.." << std::endl;
        Factory::Unit::parse(strMethod, strKey, strVal, stoi(rackID), unit); // synth, "lut1_overtones", "blaha"
        break;
    case Utils::Hash::fnv1a_hash("pattern"):
        break;
    }
}
