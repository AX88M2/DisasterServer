#ifndef DISASTERSERVER_LOBBY_HPP
#define DISASTERSERVER_LOBBY_HPP

#include "State.hpp"

#include <cstdint>
#include <unordered_map>



namespace DisasterServer
{
    class StateManager;

    class Lobby {
        Server *server = nullptr;
        StateManager *stateManager = nullptr;

        double countdown = 0;
        double prac_countdown = 0;
        uint8_t countdown_sec = 0;
        //Vote vote;

        /* Map Vote */
        uint8_t maps[3] = {};
        uint8_t votes[3] = {};

        /* Character Select */
        int8_t map = 0;
        uint16_t exe = 0;
        std::unordered_map<SurvCharacters, bool> avail;
    public:
        explicit Lobby(Server *server, StateManager *stateManager);
        ~Lobby();

        bool joined(Peer &peer);
        void tick();
        bool handle(Peer &v, Packet &packet);
    };
}
#endif //DISASTERSERVER_LOBBY_HPP
