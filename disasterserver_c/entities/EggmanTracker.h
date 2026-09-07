#ifndef EGGTRACK
#define EGGTRACK
#include "../States.h"

bool eggtrack_init(Server* server, Entity* entity);
bool eggtrack_uninit(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint16_t activ_id;
} EggTracker;
#define MakeEggTrack(x, y) ((EggTracker) { MakeEntity("eggtrack", x, y) eggtrack_init, NULL, eggtrack_uninit, 0, })

#endif