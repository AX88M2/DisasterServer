#ifndef SPIKES_H
#define SPIKES_H
#include "../States.h"

bool spike_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	uint8_t frame;
	double timer;
} SpikeController;
#define MakeSpike() ((SpikeController) { MakeEntity("spikectrl", 0, 0) NULL, spike_tick, NULL, 0, 2 * TICKSPERSEC })

#endif