#ifndef DISASTERSERVER_MAPVOTESTATE_HPP
#define DISASTERSERVER_MAPVOTESTATE_HPP

#include "Client.hpp"
#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer
{
    class Map;
    class StateController;

    class MapVoteState : public State {
        Countdown countdown { TICKS_PER_SEC };

        std::array<mapId, 3> maps = {};
        std::array<uint8_t, 3> votes = {};
    public:
        MapVoteState(Server &server, StateController &stateController);
        ~MapVoteState() override;

        void enter() override;
        void exit() override;
        bool playerJoined(Client& client) override;
        bool playerLeaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;

    private:
        void checkState();
    };
}

#endif //DISASTERSERVER_MAPVOTESTATE_HPP
