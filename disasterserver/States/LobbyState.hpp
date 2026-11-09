#pragma once

#include "State.hpp"
#include "Vote.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer
{
    class StateController;

    class LobbyState : public State {
        Countdown countdown;

        Countdown pracCountdown;

        Vote vote;
        clientId kickTarget = 0;
    public:
        LobbyState(Server &server, ContextControllers ctx);
        ~LobbyState() override;

        void enter() override;
        void exit() override;
        bool playerJoined(Client &peer) override;
        bool playerLeaved(Client &client) override;
        void tick() override;
        bool handle(Client &client, Packet &packet) override;

    private:
        bool sendCountdown();
        bool checkCountdown();
        void checkVote();
        bool cmdHandle(Client &client, clientId pid, CommandsHash hash, std::string &message);
    };
}