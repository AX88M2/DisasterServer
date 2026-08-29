#include "Server.h"
#include <entities/DTBall.h>

bool dtball_tick(Server* server, Entity* entity)
{
	DTBall* ball = (DTBall*)entity;

	if (ball->side)
	{
		ball->state += 0.015 * server->delta;

		if (ball->state >= 1)
			ball->side = 0;
	}
	else
	{
		ball->state -= 0.015 * server->delta;

		if (ball->state <= -1)
			ball->side = 1;
	}

	Packet pack;
	PacketCreate(&pack, SERVER_DTBALL_STATE);
	PacketWrite(&pack, packet_writefloat, (float)ball->state);
	server_broadcast(server, &pack, false);

	return true;
}
