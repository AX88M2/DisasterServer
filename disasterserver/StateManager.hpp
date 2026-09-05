#ifndef DISASTERSERVER_STATEMACHINE_HPP
#define DISASTERSERVER_STATEMACHINE_HPP

#include "Core/Packet.hpp"
#include "States/Lobby.hpp"

namespace DisasterServer
{
    class Peer;

    enum class States {
        LOBBY,
        MAPVOTE,
        CHARSELECT,
        GAME,
        RESULTS
    };

    class StateManager {
        Server *server = nullptr;
        States state = States::LOBBY;

        Lobby lobby;
    public:
        explicit StateManager(Server *server);
        ~StateManager();

        bool state_joined(Peer &peer);
        void state_tick();
        bool state_handle(Peer &peer, Packet &packet);

        [[nodiscard]] States getCurrentState() const { return this->state; }
    };
}

#endif //DISASTERSERVER_STATEMACHINE_HPP
