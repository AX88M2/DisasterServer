#ifndef DISASTERSERVER_LOBBY_HPP
#define DISASTERSERVER_LOBBY_HPP

#include <cstdint>

#include "Vote.hpp"
#include "Core/Types.hpp"

constexpr int COUNTDOWN = 5;
constexpr int NO_COUNTDOWN  = 5 + 1;

namespace DisasterServer
{
    class GameStateController;

    class LobbyState {
        Server *server = nullptr;
        GameStateController *controller = nullptr;

        double countdown = 0;
        double prac_countdown = 0;
        uint8_t countdown_sec = 0;
        Vote vote;
        clientId kick_target = 0;
    public:
        LobbyState(Server *server, GameStateController *controller);
        ~LobbyState() = default;

        bool init();

        bool sendCountdown();
        bool checkCountdown();
        void checkVote();

        bool joined(Client &peer);
        bool leaved(Client &peer);
        bool tick();
        bool handle(Client &client, Packet &packet);

        bool cmdHandle(Client &client, clientId pid, commandHash hash, std::string &message);
    };


}

#endif //DISASTERSERVER_LOBBY_HPP
