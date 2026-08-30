#ifndef DTBALL_H
#define DTBALL_H
#include "../States.h"

bool dtball_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	double	state;
	uint8_t side;
} DTBall;
#define MakeDTBall() ((DTBall) { MakeEntity("dtball", 0, 0) NULL, dtball_tick, NULL, 0, 0 })

#endif