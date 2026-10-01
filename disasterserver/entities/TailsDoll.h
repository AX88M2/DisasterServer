#ifndef TPROJECTILE_H
#define TPROJECTILE_H
#include "../States.h"

bool tdoll_init(Server* server, Entity* entity);
bool tdoll_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	enum
	{
		TDST_NONE,
		TDST_READY,
		TDST_FOLLOW,
		TDST_RELOC
	}			state;
	uint32_t	target;
	double		timer;
	double		velx, vely;
} TailsDoll;
#define MakeTailsDoll() ((TailsDoll) { MakeEntity("tdoll", 0, 0) tdoll_init, tdoll_tick, NULL, TDST_NONE, -1, 0, 0.0, 0.0 })

#endif