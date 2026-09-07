#include <maps/RavineMist.h>
#include <entities/RMZSlug.h>
#include <entities/RMZShard.h>
#include <States.h>
#include <CMath.h>

void shuffle(Shard* array, size_t n)
{
	if (n > 1)
	{
		for (size_t i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			Shard t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

bool rmz_checkstate(Server* server)
{
	uint8_t total = 7 - game_find(server, NULL, "shard", 7);

	Packet pack;
	PacketCreate(&pack, SERVER_RMZSHARD_STATE);
	PacketWrite(&pack, packet_write8, 3);
	PacketWrite(&pack, packet_write8, total);
	server_broadcast(server, &pack, true);

	if (server->game.time_sec <= TICKSPERSEC - 10)
	{
		uint8_t total = 7 - game_find(server, NULL, "shard", 7);
		game_bigring(server, total >= 6 ? BS_ACTIVATED : BS_DEACTIVATED);
	}

	return true;
}

bool rmz_spawnshards(Server* server, Player* player)
{
	for (uintptr_t i = 0; i < player->data[0]; i++)
	{
		Debug("shard spawned at %f %f", player->pos.x, player->pos.y);
		RAssert(game_spawn(server, (Entity*)&(MakeShard((player->pos.x + (-8 + rand() % 17)), player->pos.y, 1)), sizeof(Shard), NULL));
	}

	player->data[0] = 0;
	return true;
}

bool rmz_init(Server* server)
{
	RAssert(map_time(server, 3 * TICKSPERSEC, 20));
	RAssert(map_ring(server, 5));

	// slugs
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(1901, 392)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(2193, 392)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(2468, 392)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(1188, 860)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(2577, 1952)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(2564, 2264)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(2782, 2264)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(1441, 2264)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(884, 2264)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(988, 2004)), sizeof(SlugSpawner), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSlugSpawn(915, 2004)), sizeof(SlugSpawner), NULL));

	// shards
	Shard shards[12] = 
	{
		MakeShard(862, 248, 0),
		MakeShard(3078, 248, 0),
		MakeShard(292, 558, 0),
		MakeShard(2918, 558, 0),
		MakeShard(1100, 820, 0),
		MakeShard(980, 1188, 0),
		MakeShard(1870, 1252, 0),
		MakeShard(2180, 1508, 0),
		MakeShard(2920, 2216, 0),
		MakeShard(282, 2228, 0),
		MakeShard(1318, 1916, 0),
		MakeShard(3010, 1766, 0),
	};
	shuffle(shards, 12);

	for (int i = 0; i < 7; i++)
		RAssert(game_spawn(server, (Entity*)&shards[i], sizeof(Shard), NULL));

	return true;
}

bool rmz_tick(Server* server)
{
	if (server->game.time >= TICKSPERSEC)
	{
		if (server->game.time_sec <= TICKSPERSEC && server->game.bring_state < BS_DEACTIVATED)
			game_bigring(server, BS_DEACTIVATED);

		if (server->game.time_sec <= TICKSPERSEC - 10 && server->game.bring_state < BS_ACTIVATED)
		{
			if((7 - game_find(server, NULL, "shard", 7) >= 6))
				game_bigring(server, BS_ACTIVATED);
		}
	}

	return true;
}

bool rmz_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_RMZSLIME_HIT:
		{
			AssertOrDisconnect(v->server, v->in_game);
			if (v->server->game.end > 0)
				break;

			PacketRead(eid, packet, packet_read16, uint16_t);
			PacketRead(proj, packet, packet_read8, uint8_t);

			Slug* ent;
			if (!game_despawn(v->server, (Entity**)&ent, eid))
				break;
			
			if (proj)
			{
				free(ent);
				break;
			}

			if (ent->ring != SLUG_NORING)
			{
				if (ent->ring == SLUG_RING)
				{
					time_start(&v->plr.last_rings);
					v->plr.rings++;
					v->plr.stats.rings++;
				}

				Packet pack;
				PacketCreate(&pack, SERVER_RMZSLIME_RINGBONUS);
				PacketWrite(&pack, packet_write8, (ent->ring - 1));
				PacketWrite(&pack, packet_write8, v->plr.rings > 0);
				RAssert(packet_send(v->peer, &pack, true));
			}

			free(ent);
			break;
		}

		case CLIENT_RMZSHARD_COLLECT:
		{
			AssertOrDisconnect(v->server, v->in_game);
			if (v->server->game.end > 0)
				break;

			PacketRead(eid, packet, packet_read16, uint16_t);
			
			Shard* ent;
			if (!game_despawn(v->server, (Entity**)&ent, eid))
				break;

			// incr shard count
			v->plr.data[0]++;

			Packet pack;
			PacketCreate(&pack, SERVER_RMZSHARD_STATE);
			PacketWrite(&pack, packet_write8, 2);
			PacketWrite(&pack, packet_write16, ent->id);
			PacketWrite(&pack, packet_write16, v->id);
			server_broadcast(v->server, &pack, true);
			free(ent);

			RAssert(rmz_checkstate(v->server));
			break;
		}

		case CLIENT_PLAYER_DEATH_STATE:
		{
			if (v->server->game.end > 0)
				break;

			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			AssertOrDisconnect(v->server, v->in_game);

			PacketRead(dead, packet, packet_read8, uint8_t);
			PacketRead(rtimes, packet, packet_read8, uint8_t);

			if (dead)
				RAssert(rmz_spawnshards(v->server, &v->plr));

			RAssert(rmz_checkstate(v->server));
			break;
		}
	}
	return true;
}

bool rmz_left(PeerData* v)
{
	if(!v->server->game.started)
		return true;

	RAssert(rmz_spawnshards(v->server, &v->plr));
	RAssert(rmz_checkstate(v->server));
	return true;
}
