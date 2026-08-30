#ifndef MJASS_H
#define MJASS_H
#include "../States.h"

bool mass_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	double next_time;
	double timer;
	bool state;
} MAss;
#define MakeMAss() ((MAss) { MakeEntity("mass", 0, 0) NULL, mass_tick, NULL, 10 * TICKSPERSEC, 0.0, false })
#endif