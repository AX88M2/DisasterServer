#ifndef DISASTERSERVER_LOBBY_HPP
#define DISASTERSERVER_LOBBY_HPP

#include <cstdint>

#include "State.hpp"
#include "Vote.hpp"

constexpr int COUNTDOWN = 5;
constexpr int NO_COUNTDOWN  = 5 + 1;

namespace DisasterServer
{
    class StateController;

    class LobbyState : public State<LobbyState> {
        double countdown = 0;
        double prac_countdown = 0;
        uint8_t countdown_sec = 0;
        Vote vote;
        clientId kick_target = 0;
    public:
        LobbyState(Server *server, StateController *controller);
        ~LobbyState() override = default;

        bool init();

        bool sendCountdown();
        bool checkCountdown();
        void checkVote();

        bool joined(Client &peer) override;
        bool leaved(Client &peer) override;
        bool tick() override;
        bool handle(Client &client, Packet &packet) override;

        bool cmdHandle(Client &client, clientId pid, commandHash hash, std::string &message);

        LobbyState &get() override { return *this; }
    };


}

#endif //DISASTERSERVER_LOBBY_HPP
