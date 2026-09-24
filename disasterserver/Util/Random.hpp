#pragma once

#include <random>

namespace DisasterServer
{
    class Random {
        std::random_device random_device = {};
        std::mt19937 engine;
    public:
        Random();
        ~Random();

        int nextInt(int min, int max);
        double nextDouble(double min, double max);

        static int randInt();
    };
}
