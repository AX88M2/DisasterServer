#ifndef DISASTERSERVER_RESULTSSTATE_HPP
#define DISASTERSERVER_RESULTSSTATE_HPP

#include "State.hpp"

namespace DisasterServer
{
    class ResultsState : public State {
    public:
        ResultsState(Server &server, StateController &stateController);
        ~ResultsState() override = default;

        void init();
        bool playerJoined(Client& client) override;
        bool playerLeaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;
    };
}

#endif //DISASTERSERVER_RESULTSSTATE_HPP
