#pragma once

#include "State.hpp"
#include "Vote.hpp"
#include "Core/Constansts.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer
{
    class StateController;

    class LobbyState : public State {
        Countdown countdown { TICKS_PER_SEC };

        double pracCountdown = 0;

        Vote vote;
        clientId kickTarget = 0;
    public:
        LobbyState(Server &server, StateController &stateController);
        ~LobbyState() override;

        void enter() override;
        void exit() override;
        bool playerJoined(Client &peer) override;
        bool playerLeaved(Client &peer) override;
        void tick() override;
        bool handle(Client &client, Packet &packet) override;

    private:
        bool sendCountdown();
        bool checkCountdown();
        void checkVote();
        bool cmdHandle(Client &client, clientId pid, Commands hash, std::string &message);
    };
}