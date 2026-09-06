#ifndef DISASTERSERVER_STATEMACHINE_HPP
#define DISASTERSERVER_STATEMACHINE_HPP

#include "Core/Packet.hpp"
#include "States/LobbyState.hpp"

namespace DisasterServer
{
    class Client;

    enum class States {
        LOBBY,
        MAPVOTE,
        CHARSELECT,
        GAME,
        RESULTS
    };

    class GameStateController {
        Server *server = nullptr;
        States state = States::LOBBY;

        LobbyState lobby;
    public:
        explicit GameStateController(Server *server);
        ~GameStateController();

        bool playerJoined(Client &peer);
        void playerLeft(Client &peer);
        void tick();
        bool handle(Client &peer, Packet &packet);

        States getCurrentState() const { return this->state; }
        void setState(States state) { this->state = state; };
    };
}

#endif //DISASTERSERVER_STATEMACHINE_HPP
