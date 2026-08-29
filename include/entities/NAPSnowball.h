#ifndef NAPSNOWBALL_H
#define NAPSNOWBALL_H
#include "../States.h"

bool snowball_init(Server* server, Entity* entity);
bool snowball_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t		sid;
	bool		active;
	uint8_t		state;
	double		stage_prog;
	int8_t		dir;
	double		frame;
	double		start;
	double		vel;
	uint8_t		p_count;
	double		timer;

	float		p_move[20];
	float		p_anim[20];

} Snowball;
#define MakeSnowball(id, p_count, dir) ((Snowball) { MakeEntity("snowball", 0, 0) snowball_init, snowball_tick, NULL, id, 0, 0, 0, dir, 0, 0, 0, p_count, 0 })

bool snowball_activate(Server* server, Snowball* sb);

#endif