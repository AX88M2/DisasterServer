#ifndef DISASTERSERVER_LOBBY_HPP
#define DISASTERSERVER_LOBBY_HPP

#include <cstdint>
#include "Vote.hpp"

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

        /* Map Vote */
        uint8_t maps[3] = {};
        uint8_t votes[3] = {};

        /* Character Select */
        int8_t map = 0;
        clientId exe = 0;
        std::unordered_map<SurvCharacters, bool> avail;
    public:
        explicit LobbyState(Server *server, GameStateController *controller);
        ~LobbyState();

        bool init();

        bool sendCountdown();
        bool checkCountdown();
        void checkVote();

        bool joined(Client &peer);
        bool leaved(Client &peer);
        bool tick();
        bool handle(Client &client, Packet &packet);
    };


}

#endif //DISASTERSERVER_LOBBY_HPP
