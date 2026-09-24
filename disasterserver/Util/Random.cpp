#include "Random.hpp"

using namespace DisasterServer;

Random::Random() : engine(random_device()) {}

Random::~Random() = default;

int Random::nextInt(const int min, const int max) {
    std::uniform_int_distribution dis(min, max);
    return dis(engine);
}

double Random::nextDouble(const double min, const double max) {
    std::uniform_real_distribution dis(min, max);
    return dis(engine);
}

int Random::randInt() {
    return std::rand();
}


