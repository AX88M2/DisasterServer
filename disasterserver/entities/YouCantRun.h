#ifndef YCRCONTROLLER_H
#define YCRCONTROLLER_H
#include "../States.h"

bool ycrctrl_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	enum
	{
		YCC_NONE,
		YCC_SOME
	}			state;
	double		timer;
	uint8_t		smoke_id;
	uint8_t		activated;
} YCRController;
#define MakeYCRCtrl() ((YCRController) { MakeEntity("ycrctrl", 0, 0) NULL, ycrctrl_tick, NULL, YCC_NONE, 0, 0, 0 })

#endif