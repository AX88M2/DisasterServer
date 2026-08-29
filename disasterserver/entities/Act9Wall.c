#include "Server.h"
#include <entities/Act9Wall.h>

#define ACT9_ROOM_WIDTH (3072)

bool act9wall_init(Server* server, Entity* entity)
{
	Act9Wall* wall = (Act9Wall*)entity;
	wall->start_time = (server->game.time_sec * TICKSPERSEC + server->game.time);
	return true;
}

bool act9wall_tick(Server* server, Entity* entity)
{
	Act9Wall* wall = (Act9Wall*)entity;

	double time = (server->game.time_sec * TICKSPERSEC + server->game.time);
	double off = (double)(wall->start_time - time) / (double)wall->start_time;

	double x = wall->pos.x * off;
	double y = wall->pos.y * off;
	double wx;
	double hy;
	
	Packet pack;
	PacketCreate(&pack, SERVER_ACT9WALL_STATE);
	PacketWrite(&pack, packet_write8, wall->wid);
	PacketWrite(&pack, packet_write16, (uint16_t)x);
	PacketWrite(&pack, packet_write16, (uint16_t)y);
	server_broadcast(server, &pack, false);
	
	switch(wall->wid)
	{
		case 0:
		{
			x = -2240;
			y = y - 768;
			wx = x + (64 * 117);
			hy = y + (64 * 12);
			break;
		}

		case 1:
		{
			x = x - 2240;
			wx = x + (64 * 34);
			hy = y + (64 * 19.5);
			break;
		}

		case 2:
		{
			x = (ACT9_ROOM_WIDTH - x) + 64;
			wx = x + (64 * 34);
			hy = y + (64 * 19.5);
			break;
		}

		default:
		{
			Err("Invalid wall id!");
			return false;
		}
	}
	
	for(size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* data = (PeerData*)server->peers.ptr[i];
		if(!data)
			continue;

		if(!data->in_game)
			continue;

		if(data->plr.pos.x == 0 && data->plr.pos.y == 0)
			continue;

		if(data->plr.flags & PLAYER_DEAD)
			continue;

		if(data->plr.pos.x >= x && data->plr.pos.y >= y && data->plr.pos.x <= wx && data->plr.pos.y <= hy)
		{
			server_disconnect(server, data->peer, DR_OTHER, "Inside the wall for too long!");
			continue;
		}
	}

	return true;
}