#ifndef TPROJECTILE_H
#define TPROJECTILE_H
#include "../States.h"

bool tproj_init(Server* server, Entity* entity);
bool tproj_tick(Server* server, Entity* entity);
bool tproj_uninit(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint16_t owner;
	int8_t	 dir;
	uint8_t  exe;
	uint8_t	 charge;
	uint8_t  damage;

	double	 timer;
} TProjectile;
#define MakeTailsProj(x, y, owner, dir, exe, charge, damage) ((TProjectile) { MakeEntity("tproj", x, y) tproj_init, tproj_tick, tproj_uninit, owner, dir, exe, charge, damage, 5 * TICKSPERSEC })

#endif