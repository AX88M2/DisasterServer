#ifndef DISASTERSERVER_RESULTSSTATE_HPP
#define DISASTERSERVER_RESULTSSTATE_HPP

#include <vector>

#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Core/Types.hpp"
#include "Maps/Map.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer
{
    class ResultsState : public State {
        Countdown countdown = Countdown(TICKSPERSEC);

        mapId id;
        std::vector<clientId> leftClients;
    public:
        ResultsState(Server &server, StateController &stateController, mapId id, std::vector<clientId> &leftClients);
        ~ResultsState() override = default;

        void enter() override;
        void exit() override;
        bool playerJoined(Client& client) override;
        bool playerLeaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;
    };
}

#endif //DISASTERSERVER_RESULTSSTATE_HPP
