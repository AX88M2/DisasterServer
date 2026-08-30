#ifndef VVLAVA_H
#define VVLAVA_H
#include "../States.h"

bool lava_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t		lid;
	enum 
	{
		LV_IDLE,
		LV_MOVEDOWN,
		LV_RAISE,
		LV_MOVE,
		LV_LOWER
	}			state;
	double		timer;
	float		start;
	float		dist;
	float		vel;

} Lava;
#define MakeLava(id, start, dist) ((Lava) { MakeEntity("lava", 0, start) NULL, lava_tick, NULL, id, LV_IDLE, (20 + rand() % 5) * TICKSPERSEC, start, dist, 0 })

#endif