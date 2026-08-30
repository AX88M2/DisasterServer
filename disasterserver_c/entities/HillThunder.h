#ifndef GHZTHUNDER_H
#define GHZTHUNDER_H
#include "../States.h"

bool thunder_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	double	timer;
	bool	flag;
} Thunder;
#define MakeThunder() ((Thunder) { MakeEntity("thunder", 0, 0) NULL, thunder_tick, NULL, (15 + (rand() % 5)) * TICKSPERSEC, 0 })

#endif