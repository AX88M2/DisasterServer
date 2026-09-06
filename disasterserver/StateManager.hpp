#ifndef DISASTERSERVER_STATEMACHINE_HPP
#define DISASTERSERVER_STATEMACHINE_HPP

#include "Core/Packet.hpp"
#include "States/Lobby.hpp"

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

    class StateManager {
        Server *server = nullptr;
        States state = States::LOBBY;

        Lobby lobby;
    public:
        explicit StateManager(Server *server);
        ~StateManager();

        bool state_joined(Client &peer);
        void state_tick();
        bool state_handle(Client &peer, Packet &packet);

        [[nodiscard]] States getCurrentState() const { return this->state; }

        void setState(States state);

        void state_left(Client &peer);


    };
}

#endif //DISASTERSERVER_STATEMACHINE_HPP
