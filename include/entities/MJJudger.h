#ifndef MJJUDGER_H
#define MJJUDGER_H
#include "../States.h"

bool mjew_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	double next_time;
	double timer;
	enum
	{
		MJJ_WAIT,
		MJJ_PREPARE,
		MJJ_FIRE
	} state;
} MJew;
#define MakeMJew() ((MJew) { MakeEntity("mjud", 0, 0) NULL, mjew_tick, NULL, (10 + rand() % 5) * TICKSPERSEC, 0.0, MJJ_WAIT })
#endif