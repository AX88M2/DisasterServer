#ifndef EXELCLONE_H
#define EXELCLONE_H
#include "../States.h"

bool exclone_init(Server* server, Entity* entity);
bool exclone_uninit(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	int8_t   dir;
	uint16_t owner;
} ExellerClone;
#define MakeExellerClone(x, y, dir, owner) ((ExellerClone) { MakeEntity("exclone", x, y) exclone_init, NULL, exclone_uninit, dir, owner })

#endif