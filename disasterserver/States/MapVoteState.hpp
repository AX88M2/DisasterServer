#ifndef DISASTERSERVER_MAPVOTESTATE_HPP
#define DISASTERSERVER_MAPVOTESTATE_HPP

#include "Client.hpp"
#include "State.hpp"

namespace DisasterServer
{
    class Server;
    class StateController;

    class MapVoteState : public State<MapVoteState> {
        std::array<uint8_t, 3> maps;
        std::array<uint8_t, 3> votes;
    public:
        MapVoteState(Server *server, StateController *controller);
        ~MapVoteState() override = default;

        bool joined(Client& client) override;
        bool leaved(Client& client) override;
        bool tick() override;
        bool handle(Client& client, Packet& packet) override;

        MapVoteState &get() override { return *this; }
    };
}

#endif //DISASTERSERVER_MAPVOTESTATE_HPP
