#ifndef PFLIFT_H
#define PFLIFT_H
#include "../States.h"

typedef struct
{
	ENTITY_BODY

	uint8_t		lid;
	double		timer;
	float		start;
	float		end;
	float		speed;
	uint16_t	activator;
	bool		activated;

} PFLift;
#define MakePFLift(id, start, end) ((PFLift) { MakeEntity("pflift", 0, 0) pflift_init, pflift_tick, NULL, id, 0, start, end, 0, 0, 0 })

bool pflift_init(Server* server, Entity* entity);
bool pflift_tick(Server* server, Entity* entity);
bool pflift_activate(Server* server, PFLift* lift, uint16_t id);

#endif