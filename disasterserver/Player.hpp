#ifndef DISASTERSERVER_PLAYER_HPP
#define DISASTERSERVER_PLAYER_HPP

#include <cstdint>
#include "Util/Math.hpp"
#include "Util/Time.hpp"

typedef enum
{
    PLAYER_NONE = 0,
    PLAYER_ESCAPED = 0x1 << 0,
    PLAYER_DEAD = 0x1 << 1,
    PLAYER_DEMONIZED = 0x1 << 2,
    PLAYER_REVIVED = 0x1 << 3,
    PLAYER_CANTREVIVE = 0x1 << 4,
    PLAYER_LEFT = 0x1 << 5,
    PLAYER_KILLER = 0x1 << 6
} PlayerFlags;

typedef struct
{
    uint8_t		ready;
    uint16_t	seq;
    uint16_t	errors; /* Used as tracker for errors like lag/etc */
    uint8_t		ex_teleport;
    double		timeout;

    uint8_t 	mod_tool;
    uint32_t 	mod_tool_timer;
    uint32_t	chunk;
    TimeStamp	last_packet;

    /* Attack */
    bool		is_attacking;
    double 		attack_timer;
    TimeStamp	last_attack;

    uint16_t	ping_last;
    double		ping_total;
    double		ping_timer;
    uint16_t	rings;
    TimeStamp	last_rings;
    uint16_t	heal_rings;

    uint8_t		state;
    PlayerFlags	flags;
    uint8_t		death_timer_sec;
    double		death_timer;
    double		revival;
    int32_t		revival_init[5];

    uintptr_t	data[4]; /* Can be used as pointer/data field */
    Vector2		start_pos;
    Vector2		pos;

    struct
    {
        double		survive_time;
        double		danger_time;
        double		camp_time;

        double		braindead_time;
        bool		brain_damage;

        uint16_t	stun_time;
        uint16_t	stuns;
        uint16_t	hp_restored;
        uint16_t	rings;
        uint16_t	damage;
        uint16_t	damage_taken;
        uint16_t	kills;
    } stats;
} Player;

#endif //DISASTERSERVER_PLAYER_HPP
