#ifndef DISASTERSERVER_MAP_HPP
#define DISASTERSERVER_MAP_HPP

#include <string>
#include "Client.hpp"

namespace DisasterServer
{
    class Server;

    class Map {
        Server *server = nullptr;
        std::string name;

        uint8_t spawn_red_rings;
        uint8_t ring_count;
    public:
        Map(Server *server, const std::string &name, uint8_t spawn_red_rings, uint8_t ring_count) :
            server(server),
            name(name),
            spawn_red_rings(spawn_red_rings),
            ring_count(ring_count) {}
        virtual ~Map(){}

        virtual void init() = 0;
        virtual void tick() = 0;
        virtual void handle(Client &client, Packet& packet) = 0;
        virtual void left(Client &client) = 0;

        std::string getName() { return name; }
    };
}

#endif //DISASTERSERVER_MAP_HPP
