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
        Server *server = nullptr;
        std::string name;

        uint8_t spawn_red_rings;
        uint8_t ring_count;
    public:
        Map(Server* server, std::string name,
            uint8_t spawn_red_rings, uint8_t ring_count)
            : server(server)
            , name(std::move(name))
            , spawn_red_rings(spawn_red_rings)
            , ring_count(ring_count) {}

        virtual ~Map() = default;

        void setServer(Server* s) { server = s; }          // ← добавить
        Server* getServer() const { return server; }        // ← добавить

        virtual void init(int mapId) = 0;
        virtual void tick() = 0;
        virtual void handle(Client &client, Packet& packet) = 0;
        virtual void left(Client &client) = 0;
    };
}

#endif //DISASTERSERVER_MAP_HPP