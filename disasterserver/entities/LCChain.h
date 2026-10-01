#ifndef LCCHAIN_H
#define LCCHAIN_H
#include "../States.h"

bool	lcchain_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	double	timer;
	enum 
	{
		LCC_NONE,
		LCC_PREPARE,
		LCC_ACTIVATE
	}		state;
} LCChain;

#define MakeLCChain() ((LCChain) { MakeEntity("lcchain", 0, 0) NULL, lcchain_tick, NULL, 0, LCC_NONE })

#endif