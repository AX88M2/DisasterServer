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
    protected:
        Server* server = nullptr;
        std::string name;

        uint8_t spawn_red_rings;
        uint8_t ring_count;
    public:
        Map(Server* server, std::string name, int spawn_red_rings, int ring_count) : server(server), name(std::move(name)), spawn_red_rings(static_cast<uint8_t>(spawn_red_rings)), ring_count(static_cast<uint8_t>(ring_count)) {}

        virtual ~Map() = default;

        virtual void init() = 0;
        virtual void tick() = 0;
        virtual void handle(Client& client, Packet& packet) = 0;
        virtual void left(Client& client) = 0;

        const std::string& getName() const { return name; }
    };
}

#endif