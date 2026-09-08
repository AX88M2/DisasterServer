#ifndef DISASTERSERVER_MAPVOTESTATE_HPP
#define DISASTERSERVER_MAPVOTESTATE_HPP

#include "Client.hpp"
#include "Core/State.hpp"

namespace DisasterServer
{
    class Server;
    class StateController;

    class MapVoteState : public State {
        std::array<uint8_t, 3> maps;
        std::array<uint8_t, 3> votes;
    public:
        MapVoteState(Server *server, StateController *controller);
        ~MapVoteState() override = default;

        bool joined(Client& client) override;
        bool leaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

        MapVoteState &get() { return *this; }
    };
}

#endif //DISASTERSERVER_MAPVOTESTATE_HPP
