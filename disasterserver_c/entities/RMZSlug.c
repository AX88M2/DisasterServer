#include <entities/RMZSlug.h>
#include <CMath.h>

void slug_face(Slug* slug, uint8_t side)
{
	if (side)
	{
		switch (slug->ring)
		{
			case SLUG_NORING:
				slug->state = SLUG_NONERIGHT;
				break;

			case SLUG_RING:
				slug->state = SLUG_RINGRIGHT;
				break;

			case SLUG_REDRING:
				slug->state = SLUG_REDRINGRIGHT;
				break;
		}
	}
	else
	{
		switch (slug->ring)
		{
		case SLUG_NORING:
			slug->state = SLUG_NONELEFT;
			break;

		case SLUG_RING:
			slug->state = SLUG_RINGLEFT;
			break;

		case SLUG_REDRING:
			slug->state = SLUG_REDRINGLEFT;
			break;
		}
	}
}

bool slug_init(Server* server, Entity* entity)
{
	Slug* slug = (Slug*)entity;

	int num = rand() % 100;
	if (num < 50)
		slug->ring = SLUG_NORING;
	else if (num >= 40 && num < 90)
		slug->ring = SLUG_RING;
	else
		slug->ring = SLUG_REDRING;

	slug->sX = slug->pos.x;
	slug->sY = slug->pos.y;

	// make it face random dir
	slug_face(slug, rand() % 2);

	Packet pack;
	PacketCreate(&pack, SERVER_RMZSLIME_STATE);
	PacketWrite(&pack, packet_write8,	0);
	PacketWrite(&pack, packet_write16,	slug->id);
	PacketWrite(&pack, packet_write16,	(uint16_t)slug->pos.x);
	PacketWrite(&pack, packet_write16,	(uint16_t)slug->pos.y);
	PacketWrite(&pack, packet_write8,	(uint8_t)slug->state);
	server_broadcast(server, &pack, true);

	return true;
}

bool slug_tick(Server* server, Entity* entity)
{
	Slug* slug = (Slug*)entity;

	switch (slug->state)
	{
		case SLUG_NONELEFT:
		case SLUG_RINGLEFT:
		case SLUG_REDRINGLEFT:
			slug->pos.x -= (float)server->delta;
			if (slug->pos.x <= slug->sX - 100)
				slug_face(slug, 1);
			break;

		case SLUG_NONERIGHT:
		case SLUG_RINGRIGHT:
		case SLUG_REDRINGRIGHT:
			slug->pos.x += (float)server->delta;
			if (slug->pos.x >= slug->sX + 100)
				slug_face(slug, 0);
			break;
	}

	Packet pack;
	PacketCreate(&pack, SERVER_RMZSLIME_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write16, slug->id);
	PacketWrite(&pack, packet_write16, (uint16_t)slug->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)slug->pos.y);
	PacketWrite(&pack, packet_write8, (uint8_t)slug->state);
	server_broadcast(server, &pack, false);

	return true;
}

bool slug_uninit(Server* server, Entity* entity)
{
	Packet pack;
	PacketCreate(&pack, SERVER_RMZSLIME_STATE);
	PacketWrite(&pack, packet_write8, 2);
	PacketWrite(&pack, packet_write16, entity->id);
	server_broadcast(server, &pack, true);

	// find slug spawner and free it
	SlugSpawner* spawners[11];

	int found = game_find(server, (Entity**)spawners, "slugspawn", 11);
	for (int i = 0; i < found; i++)
	{
		if (spawners[i]->slug == entity->id)
		{
			Debug("removed slug from %d", spawners[i]->id);
			spawners[i]->slug = 0;
			break;
		}
	}

	return true;
}

bool slugspawn_tick(Server* server, Entity* entity)
{
	SlugSpawner* spawn = (SlugSpawner*)entity;
	if (spawn->slug > 0)
		return true;

	if (spawn->offset > 0)
	{
		spawn->offset -= server->delta;
		return true;
	}

	spawn->timer += server->delta;
	if (spawn->timer >= 15 * TICKSPERSEC)
	{
		Slug* slug;
		RAssert(game_spawn(server, (Entity*)&(MakeSlug(spawn->pos.x, spawn->pos.y)), sizeof(Slug), (Entity**)&slug));
		
		spawn->timer = 0;
		spawn->slug = slug->id;
		spawn->offset = (double)((rand() % 2) * TICKSPERSEC);
	}

	return true;
}
