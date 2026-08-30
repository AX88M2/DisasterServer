#ifndef MJLAVA_H
#define MJLAVA_H
#include "../States.h"

bool mlava_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	enum
	{
		MLV_IDLE,
		MLV_MOVEDOWN,
		MLV_RAISE,
		MLV_MOVE,
		MLV_LOWER
	}			state;
	double		timer;
	float		start;
	float		dist;
	float		vel;

} MLava;
#define MakeMLava(start, dist) ((MLava) { MakeEntity("mlava", 0, start) NULL, mlava_tick, NULL, MLV_IDLE, 5 * TICKSPERSEC, start, dist, 0 })

#endif