#ifndef DISASTERSERVER_MAP_HPP
#define DISASTERSERVER_MAP_HPP

#include <cstdint>
#include <string>

namespace DisasterServer
{
    class Server;
    class Client;
    class Packet;

    class Map {
        std::string name;

        int spawnRedRings;
        int ringCount;
    public:
        Map(std::string name, const int spawnRedRings, const int ringCount) : name(std::move(name)),
            spawnRedRings(spawnRedRings), ringCount(ringCount) {}

        virtual ~Map() = default;

        virtual void init() = 0;
        virtual void tick() = 0;
        virtual void handle(Client& client, Packet& packet) = 0;
        virtual void left(Client& client) = 0;

        const std::string& getName() const { return name; }
    };
}

#endif