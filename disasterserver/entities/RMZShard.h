#ifndef RMZSHARD_H
#define RMZSHARD_H
#include "../States.h"

bool shard_init(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t spawned;
} Shard;
#define MakeShard(x, y, spawned) ((Shard) { MakeEntity("shard", x, y) shard_init, NULL, NULL, spawned })

#endif