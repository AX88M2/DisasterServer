#ifndef HDDOOR_H
#define HDDOOR_H
#include "../States.h"

bool	hddoor_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t		state;
	double		timer;
} HDDoor;

#define MakeHDDoor() ((HDDoor) { MakeEntity("hddoor", 0, 0) NULL, hddoor_tick, NULL, 0, 0 })
bool	hddoor_toggle(Server* server, HDDoor* door);

#endif