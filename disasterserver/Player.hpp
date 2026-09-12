#ifndef DISASTERSERVER_PLAYER_HPP
#define DISASTERSERVER_PLAYER_HPP

#include "Core/Time.hpp"
#include "Core/Types.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer
{
    class Player {
        uint8_t	ready;
        uint16_t seq;
        uint16_t errors; /* Used as tracker for errors like lag/etc */
        uint8_t	ex_teleport;
        double timeout;

        uint8_t mod_tool;
        uint32_t mod_tool_timer;
        uint32_t chunk;
        TimeStamp last_packet;

        /* Attack */
        bool is_attacking;
        double attack_timer;
        TimeStamp last_attack;

        uint16_t ping_last;
        double ping_total;
        double ping_timer;
        uint16_t rings;
        TimeStamp last_rings;
        uint16_t heal_rings;

        uint8_t	state;
    public:
        Player();
        ~Player();
    };
}

#endif //DISASTERSERVER_PLAYER_HPP
