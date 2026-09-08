#ifndef VVVASE_H
#define VVVASE_H
#include "../States.h"

typedef struct
{
	ENTITY_BODY

	uint8_t vid;
	uint8_t type;
} Vase;
#define MakeVase(id) ((Vase) { MakeEntity("vase", 0, 0) NULL, NULL, NULL, id, (rand() % 4) })

#endif