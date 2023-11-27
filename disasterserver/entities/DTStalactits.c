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
	server_broadcast(server, &pack);

	return true;
}

bool dtst_tick(Server* server, Entity* entity)
{
	DTStalactits* titi = (DTStalactits*)entity;

	if (titi->state)
	{
		titi->vel += 0.164f * server->delta;
		titi->pos.y += titi->vel * server->delta;

		Packet pack;
		PacketCreate(&pack, SERVER_DTASS_STATE);
		PacketWrite(&pack, packet_write8, titi->sid);
		PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.x);
		PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.y);
		game_broadcast(server, &pack);
	}
	else
	{
		if (titi->timer > 0)
			titi->timer -= server->delta;

		if (titi->timer <= TICKSPERSEC && !titi->show)
		{
			Packet pack;
			PacketCreate(&pack, SERVER_DTASS_STATE);
			PacketWrite(&pack, packet_write8, 0);
			PacketWrite(&pack, packet_write8, titi->sid);
			PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.x);
			PacketWrite(&pack, packet_write16, (uint16_t)titi->pos.y);
			server_broadcast(server, &pack);

			titi->show = 1;
		}

		if (titi->timer <= 0)
		{
			for (size_t i = 0; i < server->game.players.capacity; i++)
			{
				Player* plr = (Player*)server->game.players.ptr[i];
				if (!plr)
					continue;

				if (plr->flags & PLAYER_DEAD)
					continue;

				float dist = plr->pos.y - titi->pos.y;
				if (dist > 0 && dist <= 336 && plr->pos.x >= titi->pos.x && plr->pos.x <= titi->pos.x + 80)
				{
					titi->vel = 0;

					Packet pack;
					PacketCreate(&pack, SERVER_DTASS_STATE);
					PacketWrite(&pack, packet_write8, 2);
					PacketWrite(&pack, packet_write8, titi->sid);
					server_broadcast(server, &pack);

					titi->state = 1;
					break;
				}
			}
		}
	}

	return true;
}

bool dtst_activate(Server* server, DTStalactits* tits)
{
	tits->show = 0;
	tits->state = 0;
	tits->timer = (rand() % 5 + 25.0) * TICKSPERSEC;
	tits->pos.y = tits->sy;
	tits->vel = 0;

	Packet pack;
	PacketCreate(&pack, SERVER_DTASS_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write8, tits->sid);
	server_broadcast(server, &pack);

	return true;
}
