#include "Server.h"
#include <entities/TailsDoll.h>
#include <CMath.h>

Vector2 spots[11] =
{
	{ 177, 944 },
	{ 1953, 544 },
	{ 3279, 224 },
	{ 4101, 544 },
	{ 4060, 1264 },
	{ 3805, 1824 },
	{ 2562, 1584 },
	{ 515,  1824 },
	{ 2115, 1056 },
	{ 984,  1184 },
	{ 1498, 1504 }
};
	
void tdoll_find_spot(Server* server, TailsDoll* doll)
{
	Vector2 new_spots[11];
	int     new_count = 0;

	for (int i = 0; i < 11; i++)
	{
		bool can_use = true; 
		for (size_t j = 0; j < server->peers.capacity; j++)
		{
			PeerData* data = (PeerData*)server->peers.ptr[j];
			if (!data)
				continue;

			if(!data->in_game)
				continue;

			if (vector2_dist(&spots[i], &data->plr.pos) < 480.f)
			{
				can_use = false;
				break;
			}
		}

		if(can_use)
			new_spots[new_count++] = spots[i];
	}

	if (new_count > 0)
	{
		int ball = rand() % new_count;
		Vector2 spot = new_spots[ball];
		doll->pos.x = spot.x;
		doll->pos.y = spot.y;

		Debug("Tails doll found spot %d at %f %f", ball, doll->pos.x, doll->pos.y);
	}
	else
	{
		Vector2 spot = spots[rand() % 11];
		doll->pos.x = spot.x;
		doll->pos.y = spot.y;

		Debug("Tails doll didn't find a spot, using %f %f", doll->pos.x, doll->pos.y);
	}
}

bool tdoll_is_vaild_target(Server* server, TailsDoll* doll)
{
	if (doll->target == -1)
		return false;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* data = (PeerData*)server->peers.ptr[i];
		if (!data)
			continue;

		if(!data->in_game)
			continue;

		if (data->id == server->game.exe)
			continue;

		if (data->plr.flags & PLAYER_DEAD)
			continue;

		if (data->plr.flags & PLAYER_DEMONIZED)
			continue;

		if (data->plr.flags & PLAYER_ESCAPED)
			continue;

		if (data->id == doll->target)
			return true;
	}

	return false;
}

bool tdoll_find_target(Server* server, TailsDoll* doll)
{
	Vector2 pos = { doll->pos.x, doll->pos.y };

	// scan for people
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* data = (PeerData*)server->peers.ptr[i];
		if (!data)
			continue;

		if(!data->in_game)
			continue;

		if (data->id == server->game.exe)
			continue;

		if (data->plr.flags & PLAYER_DEAD)
			continue;

		if (data->plr.flags & PLAYER_DEMONIZED)
			continue;

		if (data->plr.flags & PLAYER_ESCAPED)
			continue;

		if (vector2_dist(&data->plr.pos, &pos) < 130)
		{
			doll->target = data->id;
			return true;
		}
	}

	return false;
}

bool tdoll_init(Server* server, Entity* entity)
{
	TailsDoll* doll = (TailsDoll*)entity;
	tdoll_find_spot(server, doll);

	Packet pack;
	PacketCreate(&pack, SERVER_DTTAILSDOLL_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write16, (uint16_t)doll->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)doll->pos.y);
	PacketWrite(&pack, packet_write8, (uint8_t)doll->state);
	server_broadcast(server, &pack, true);

	return true;
}

bool tdoll_tick(Server* server, Entity* entity)
{
	TailsDoll* doll = (TailsDoll*)entity;
	switch (doll->state)
	{
		case TDST_NONE:
		{
			if (tdoll_find_target(server, doll))
			{
				doll->timer = (1 + (rand() % 2) * 0.5) * TICKSPERSEC;
				doll->state = TDST_READY;

				Packet pack;
				PacketCreate(&pack, SERVER_DTTAILSDOLL_STATE);
				PacketWrite(&pack, packet_write8, 2);
				packet_send_id(server, doll->target, &pack, true);
			}
			break;
		}

		case TDST_READY:
		{
			doll->timer -= server->delta;
			if (doll->timer <= 0)
			{
				// Send that we are following now
				Packet pack;
				PacketCreate(&pack, SERVER_DTTAILSDOLL_STATE);
				PacketWrite(&pack, packet_write8, 3);
				packet_send_id(server, doll->target, &pack, true);

				doll->state = TDST_FOLLOW;
			}
			break;
		}

		case TDST_FOLLOW:
		{
			// keep searching
			tdoll_find_target(server, doll);

			if (!tdoll_is_vaild_target(server, doll))
			{
				doll->state = TDST_RELOC;
				break;
			}

			PeerData* data = server_find_peer(server, doll->target);
			if (!data || !data->in_game)
			{
				doll->state = TDST_RELOC;
				break;
			}

			if (abs((int)(data->plr.pos.x - doll->pos.x)) >= 4)
			{
				doll->velx += sign((int)data->plr.pos.x - doll->pos.x) * 0.512 * server->delta;
				doll->velx = fmin(fmax(doll->velx, -5), 5);
			}

			if (abs((int)(data->plr.pos.y - doll->pos.y)) >= 5)
			{
				doll->vely += sign((int)data->plr.pos.y - doll->pos.y) * 0.480 * server->delta;
				doll->vely = fmin(fmax(doll->vely, -5), 5);
			}

			doll->pos.x += doll->velx * (float)server->delta;
			doll->pos.y += doll->vely * (float)server->delta;

			if (vector2_dist(&doll->pos, &data->plr.pos) < 12)
			{
				Packet pack;
				PacketCreate(&pack, SERVER_DTTAILSDOLL_STATE);
				PacketWrite(&pack, packet_write8, 1);
				packet_send_id(server, doll->target, &pack, true);

				doll->state = TDST_RELOC;
				break;
			}

			break;
		}

		case TDST_RELOC:
		{
			tdoll_find_spot(server, doll);
			doll->state = TDST_NONE;
			break;
		}
	}

	Packet pack;
	PacketCreate(&pack, SERVER_DTTAILSDOLL_STATE);
	PacketWrite(&pack, packet_write16, (uint16_t)doll->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)doll->pos.y);
	PacketWrite(&pack, packet_write8, doll->state);
	server_broadcast(server, &pack, false);

	return true;
}
