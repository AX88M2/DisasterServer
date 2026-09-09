#ifndef DISASTERSERVER_MAPVOTESTATE_HPP
#define DISASTERSERVER_MAPVOTESTATE_HPP

#include "Client.hpp"
#include "State.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer
{
    class Server;
    class StateController;

    class MapVoteState : public State {
        Countdown countdown;

        std::array<uint8_t, 3> maps = {};
        std::array<uint8_t, 3> votes = {};
    public:
        MapVoteState(Server *server, StateController *controller);
        ~MapVoteState() override = default;

        void init();
        bool joined(Client& client) override;
        bool leaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

    private:
        void checkState();
    };
}

#endif //DISASTERSERVER_MAPVOTESTATE_HPP
