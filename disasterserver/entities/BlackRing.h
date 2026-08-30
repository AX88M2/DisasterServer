#ifndef BRING_H
#define BRING_H
#include "../States.h"
#include <stdint.h>

bool bring_init(Server* server, Entity* entity);
bool bring_uninit(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY
} BRing;
#define MakeBlackRing(x, y) ((BRing) { MakeEntity("bring", x, y) bring_init, NULL, bring_uninit})
#define MAP_BRING INT16_MAX

#endif