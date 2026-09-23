#pragma once

#include <string>
#include "Core/Constansts.hpp"

namespace DisasterServer
{
    class Server;
    class Client;
    class Packet;
    class GameState;

    struct MapProperties {
        int time = 3 * TICKS_PER_SEC;
        float mul = 20;
        int ringCoff = 5;
    };

    class Map {
        std::string name;

        MapProperties countdown;
        int spawnRedRings;
        int ringCount;
    public:
        Map(std::string name, const int spawnRedRings, const int ringCount) : name(std::move(name)),
            spawnRedRings(spawnRedRings), ringCount(ringCount) {}

        virtual ~Map() = default;

        virtual void init(GameState& game) = 0;
        virtual void tick() = 0;
        virtual void handle(Client& client, Packet& packet) = 0;
        virtual void left(Client& client) = 0;
        virtual MapProperties getMapTime() const = 0;

        const std::string& getName() const { return name; }

        int getSpawnRedRings() const { return spawnRedRings; }
        int getRingCount() const { return ringCount; }
    };
}