#ifndef PLAYER_H
#define PLAYER_H
#include <io/TcpListener.h>
#include <io/UdpListener.h>
#include <CMath.h>

typedef enum
{
	PLAYER_NONE = 0,
	PLAYER_ESCAPED = 0x1 << 0,
	PLAYER_DEAD = 0x1 << 1,
	PLAYER_DEMONIZED = 0x1 << 2,
	PLAYER_REVIVED = 0x1 << 3,
	PLAYER_CANTREVIVE = 0x1 << 4
} PlayerFlags;

typedef struct
{
	uint8_t		ready;
	uint16_t	seq;
	uint16_t	id;
	uint16_t	errors; /* Used as tracker for errors like lag/etc */
	uint8_t		ex_teleport;
	uint8_t		rings;
	double		timeout;

	PlayerFlags	flags;
	uint8_t		death_timer_sec;
	double		death_timer;
	double		revival;
	int32_t		revival_init[5];

	uintptr_t	data[4]; /* Can be used as pointer/data field */
	Vector2		pos;
	IpAddr		addr;
} Player;

#define SET_FLAG(x, flag) x |= (flag)
#define DEL_FLAG(x, flag) x = x & ~(flag)

struct Server;
bool player_add_error (struct Server* server, Player* player, uint16_t by);

#endif