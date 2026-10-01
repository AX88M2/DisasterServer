#ifndef DTSTALACTITS_H
#define DTSTALACTITS_H
#include "../States.h"

bool dtst_init(Server* server, Entity* entity);
bool dtst_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t		sid;
	bool		state;
	bool		show;
	uint16_t	sx, sy;
	double		timer;
	float		vel;
} DTStalactits;
#define MakeDTStalactiti(id, x, y) ((DTStalactits) { MakeEntity("dttits", x, y) dtst_init, dtst_tick, NULL, id, 0, 1, x, y, 0, 0 })

bool dtst_activate(Server* server, DTStalactits* tits);

#endif