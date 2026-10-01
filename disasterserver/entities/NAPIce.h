#ifndef NAPICE_H
#define NAPICE_H
#include "../States.h"

bool ice_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t		iid;
	bool		activated;
	double		timer;

} Ice;
#define MakeIce(id) ((Ice) { MakeEntity("ice", 0, 0) NULL, ice_tick, NULL, id, 0, 0 })

bool ice_activate(Server* server, Ice* ice);

#endif