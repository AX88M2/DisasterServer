#ifndef RING_H
#define RING_H
#include "../States.h"

bool ring_init(Server* server, Entity* entity);
bool ring_uninit(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t rid;
	uint8_t red;
} Ring;
#define MakeRing() ((Ring) { MakeEntity("ring", 0, 0) ring_init, NULL, ring_uninit, 0, 0 })

#endif