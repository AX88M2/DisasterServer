#ifndef TCACID_H
#define TCACID_H
#include "../States.h"

bool acid_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t		acid_id;
	uint8_t		activated;
	double		timer;
} Acid;
#define MakeAcid() ((Acid) { MakeEntity("acid", 0, 0) NULL, acid_tick, NULL, 0, 0, 0.0 })

#endif