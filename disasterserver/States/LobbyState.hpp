#ifndef DISASTERSERVER_LOBBY_HPP
#define DISASTERSERVER_LOBBY_HPP

#include <cstdint>

#include "State.hpp"
#include "Vote.hpp"
#include "Core/Constansts.hpp"
#include "Util/Countdown.hpp"

constexpr int START_COUNTDOWN = 5;
constexpr int NO_COUNTDOWN  = START_COUNTDOWN + 1;

namespace DisasterServer
{
    class StateController;

    class LobbyState : public State {
        Countdown countdown { TICKSPERSEC };
        double pracCountdown = 0;
        Vote vote;
        clientId kickTarget = 0;
    public:
        LobbyState(Server &server, StateController &stateController);
        ~LobbyState() override = default;

        void enter() override;
        void exit() override;
        bool joined(Client &peer) override;
        bool leaved(Client &peer) override;
        void tick() override;
        bool handle(Client &client, Packet &packet) override;

    private:
        bool sendCountdown();
        bool checkCountdown();
        void checkVote();
        bool cmdHandle(Client &client, clientId pid, commandHash hash, std::string &message);
    };
}

#endif