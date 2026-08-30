#ifndef MAPS_H
#define MAPS_H
#include <Server.h>

typedef struct
{
	const char* name;

	struct
	{
		bool (*init)(Server*);
		bool (*tick)(Server*);
		bool (*tcp_msg)(PeerData*, Packet*);
		bool (*left)(PeerData*);
	} cb;

	uint8_t  spawn_red_rings;
	uint8_t  ring_count;
} Map;

#define MAP_COUNT 20
extern Map g_mapList[MAP_COUNT+1];

bool map_init 	(Server* server);
bool map_tick 	(Server* server);
bool map_tcpmsg (PeerData* v, Packet* packet);
bool map_left 	(PeerData* v);

bool map_time (Server* server, double seconds, float mul);
bool map_ring (Server* server, int ringcoff);

#endif
