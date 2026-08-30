#ifndef WDLATERN_H
#define WDLATERN_H
#include <stdlib.h>
#include "../States.h"

bool latern_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	bool side;
	double timer;
	uint8_t	lid;
	uint16_t time;
} Latern;
#define MakeLatern() ((Latern) { MakeEntity("latrn", 0, 0) NULL, latern_tick, NULL, false, 0.0, 0, (7 + rand() % 2) })

#endif