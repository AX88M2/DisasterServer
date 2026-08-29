#ifndef KAFSPEEDBOX_H
#define KAFSPEEDBOX_H
#include "../States.h"

bool kafbox_init(Server* server, Entity* entity);
bool kafbox_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t	 nid;
	double	 timer;
	bool	 activated;
} KafBox;
#define MakeKafBox(nid) ((KafBox) { MakeEntity("kafbox", 0, 0) kafbox_init, kafbox_tick, NULL, nid, 0, 0, })

bool kafbox_activate(Server* server, KafBox* box, uint16_t pid, uint8_t is_proj);

#endif