#ifndef DISASTERSERVER_LOBBY_HPP
#define DISASTERSERVER_LOBBY_HPP

#include <cstdint>

#include "State.hpp"
#include "Vote.hpp"

constexpr int START_COUNTDOWN = 5;
constexpr int NO_COUNTDOWN  = START_COUNTDOWN + 1;

namespace DisasterServer
{
    class StateController;

    class LobbyState : public State {
        double countdown = 0;
        double pracCountdown = 0;
        uint8_t countdownSec = 0;
        Vote vote;
        clientId kick_target = 0;
    public:
        static constexpr States STATE_ID = States::LOBBY;

        LobbyState(Server *server, StateController *controller);
        ~LobbyState() override = default;

        void init();

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