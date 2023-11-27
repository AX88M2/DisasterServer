#ifndef MAPS_H
#define MAPS_H
#include "Server.h"

typedef struct
{
	const char* name;

	struct
	{
		bool (*init)(Server*);
		bool (*tick)(Server*);
		bool (*tcp_msg)(PeerData*, Packet*);
		bool (*left)(PeerData*);
		bool (*uninit)(Server*);
	} cb;

	uint8_t  spawn_red_rings;
	uint8_t  ring_count;
} Map;

extern Map g_mapList[19];
#define MAP_COUNT 18

bool map_init 	(Server* server);
bool map_uninit (Server* server);
bool map_tick 	(Server* server);
bool map_tcpmsg (PeerData* v, Packet* packet);
bool map_left 	(PeerData* v);

bool map_time (Server* server, double seconds, float mul);
bool map_ring (Server* server, int ringcoff);

#endif