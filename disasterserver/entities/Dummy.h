#ifndef DUMMY_H
#define DUMMY_H
#include "../States.h"

bool dummy_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	double vel;
} Dummy;
#define MakeDummy() ((Dummy) { MakeEntity("dummy", 1616, 2608) NULL, dummy_tick, NULL, 0 })

void dummy_activate(Dummy* dummy, int8_t dir);

#endif