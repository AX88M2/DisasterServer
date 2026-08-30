#ifndef ACT9WALL_H
#define ACT9WALL_H
#include "../States.h"

bool act9wall_init(Server* server, Entity* entity);
bool act9wall_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t	 wid;
	double	 start_time;
} Act9Wall;
#define MakeAct9Wall(wid, x, y) ((Act9Wall) { MakeEntity("act9wall", x, y) act9wall_init, act9wall_tick, NULL, wid, 0 })

#endif