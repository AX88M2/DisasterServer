#include "Server.h"
#include <entities/DTStalactits.h>

bool dtst_init(Server* server, Entity* entity)
{
	DTStalactits* titi = (DTStalactits*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_DTASS_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write8, titi->sid);
	PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.y);
	server_broadcast(server, &pack, true);

	return true;
}

bool dtst_tick(Server* server, Entity* entity)
{
	DTStalactits* titi = (DTStalactits*)entity;
	if (titi->state)
	{
		titi->vel += 0.164f * (float)server->delta;
		titi->pos.y += titi->vel * (float)server->delta;

		Packet pack;
		PacketCreate(&pack, SERVER_DTASS_STATE);
		PacketWrite(&pack, packet_write8, titi->sid);
		PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.x);
		PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.y);
		server_broadcast(server, &pack, false);
	}
	else
	{
		if (!titi->show)
		{
			if (titi->timer > TICKSPERSEC)
				titi->timer -= server->delta;

			if (titi->timer <= TICKSPERSEC)
			{
				Packet pack;
				PacketCreate(&pack, SERVER_DTASS_STATE);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write8, titi->sid);
				PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.x);
				PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.y);
				server_broadcast(server, &pack, true);

				titi->show = true;
			}
		}
		else
		{
			if (titi->timer > 0)
				titi->timer -= server->delta;
		}

		if (titi->timer <= 0)
		{
			for (size_t i = 0; i < server->peers.capacity; i++)
			{
				PeerData* data = (PeerData*)server->peers.ptr[i];
				if (!data)
					continue;

				if(!data->in_game)
					continue;

				if (data->plr.flags & PLAYER_DEAD)
					continue;

				float dist = data->plr.pos.y - titi->pos.y;
				if (dist > 0 && dist <= 336 && data->plr.pos.x >= titi->pos.x && data->plr.pos.x <= titi->pos.x + 80)
				{
					titi->vel = 0;

					Packet pack;
					PacketCreate(&pack, SERVER_DTASS_STATE);
					PacketWrite(&pack, packet_write8, 2);
					PacketWrite(&pack, packet_write8, titi->sid);
					server_broadcast(server, &pack, true);

					titi->state = true;
					break;
				}
			}
		}
	}

	return true;
}

bool dtst_activate(Server* server, DTStalactits* tits)
{
	if(!tits->state)
		return true;
		
	tits->show = false;
	tits->state = false;
	tits->timer = (25.0 + rand() % 5 ) * TICKSPERSEC;
	tits->pos.y = tits->sy;
	tits->vel = 0;

	Packet pack;
	PacketCreate(&pack, SERVER_DTASS_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write8, tits->sid);
	server_broadcast(server, &pack, true);

	return true;
}
