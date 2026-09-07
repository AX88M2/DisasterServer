#ifndef NPCONTROLLER_H
#define NPCONTROLLER_H
#include "../States.h"

bool npctrl_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	enum
	{
		NPC_NONE,
		NPC_PREPARE
	}		state;
	uint8_t stage;
	double	timer;
	bool	balls;
} NPController;
#define MakeNPCtrl() ((NPController) { MakeEntity("npctrl", 0, 0) NULL, npctrl_tick, NULL, NPC_NONE, 0, 0, 0 })

#endif