#ifndef DISASTERSERVER_MAPVOTESTATE_HPP
#define DISASTERSERVER_MAPVOTESTATE_HPP

#include "Client.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{
    class Server;
    class GameStateController;

    class MapVoteState {
        Server *server;
        GameStateController *controller;

        uint8_t maps[3] = {};
        uint8_t votes[3] = {};
    public:
        MapVoteState(Server *server, GameStateController *controller);
        ~MapVoteState() = default;

        bool joined(Client& client);
        bool leaved(Client& client);
        void tick();
        bool handle(Client& client, Packet& packet);
    };
}

#endif //DISASTERSERVER_MAPVOTESTATE_HPP
