#ifndef LCEYE_H
#define LCEYE_H
#include "../States.h"

bool	lceye_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t		eye_id;
	uint16_t	use_id;
	bool		used;
	uint8_t		charge;
	uint16_t	target;

	double		cooldown;
	double		timer;

} LCEye;

#define MakeLCEye(id) ((LCEye) { MakeEntity("lceye", 0, 0) NULL, lceye_tick, NULL, id, 0, 0, 100, 0, 0, 0 })
bool  lceye_update(Server* server, LCEye* eye);

#endif