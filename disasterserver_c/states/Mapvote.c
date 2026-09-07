#include "Server.h"
#include <States.h>
#include <Maps.h>
#include <CMath.h>
#include <Colors.h>
#include <time.h>

bool mapvote_check_state(Server* server)
{
	RAssert(server);

	uint8_t count = 0;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->in_game && peer->voted)
			count++;
	}

	if (count >= server_ingame(server))
	{
		if (server->lobby.countdown_sec > 3)
		{
			server->lobby.countdown = 0;
			server->lobby.countdown_sec = 4;
		}
	}

	return true;
}

bool mapvote_state_handle(PeerData* v, Packet* packet)
{
	// Read header
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	Packet pack;
	bool res = true;

	switch (type)
	{			
		case CLIENT_VOTE_REQUEST:
		{
			if(!v->in_game)
				break;

			PacketRead(map, packet, packet_read8, uint8_t);
			AssertOrDisconnect(v->server, !v->voted);
			AssertOrDisconnect(v->server, map >= 0);
			AssertOrDisconnect(v->server, map < 3);

			v->server->lobby.votes[map]++;
			v->voted = true;

			PacketCreate(&pack, SERVER_VOTE_SET);
			for (int i = 0; i < 3; i++)
				PacketWrite(&pack, packet_write8, v->server->lobby.votes[i]);

			Info("%s " LOG_RST "(id %d) voted for [" LOG_BLU "%s" LOG_RST "]!", v->nickname.value, v->id, g_mapList[v->server->lobby.maps[map]].name);
			server_broadcast(v->server, &pack, true);
			mapvote_check_state(v->server);
			break;
		}

		case CLIENT_CHAT_MESSAGE:
		{
			PacketRead(pid, packet, packet_read16, uint16_t);
			PacketRead(msg, packet, packet_readstr, String);
			AssertOrDisconnect(v->server, string_length(&msg) <= 40);

			v->timeout = 0;
			Info("%s " LOG_RST "(id %d): %s", v->nickname.value, v->id, msg.value);
			if (!server_cmd_handle(v->server, server_cmd_parse(&msg), v, &msg))
				server_broadcast_ex(v->server, packet, true, v->id);

			break;
		}

		default:
			RAssert(server_msg_handle(v->server, type, v, packet));
			break;
	}

	return res;
}

bool mapvote_state_tick(Server* server)
{
	bool res = true;

	if (server->lobby.countdown <= 0)
	{
		server->lobby.countdown += TICKSPERSEC;
	
		if (--server->lobby.countdown_sec == 0)
		{
			//choose the map
			int8_t indeces[3] = { -1, -1, -1 };
			int count = 0;

			int largest = 0;
			for (int i = 0; i < 3; i++)
			{
				uint8_t values = server->lobby.votes[i];
				if (values > largest)
				{
					largest = values;
					count = 0;
				}

				if (values == largest)
				{
					indeces[count] = server->lobby.maps[i];
					count++;
				}
			}

			// Find winner
			int8_t won = indeces[rand() % count];
			server->last_map = won;
			
			// Decrease pickrate
			if ((server->map_pickrates[won] -= 255) < 0)
				server->map_pickrates[won] = 0;

			// Decrease pickrate for other maps
			for (int8_t i = 0; i < 3; i++)
			{
				if ((server->map_pickrates[server->lobby.maps[i]] -= 25) < 0)
					server->map_pickrates[server->lobby.maps[i]] = 0;
			}

			Debug("Pickrates:");
			// Increase other map's pickrate
			for (int8_t i = 0; i < MAP_COUNT; i++)
			{
				Debug("		%d: %d", i, server->map_pickrates[i]);
				if (i == won)
					continue;

				server->map_pickrates[i] += 25;
				if (server->map_pickrates[i] >= 255)
					server->map_pickrates[i] = 255;
			}

			Info(LOG_YLW "Map is [" LOG_BLU "%s" LOG_RST "]", g_mapList[won].name);

			return charselect_init(won, server) || lobby_init(server);
		}

		Packet pack;
		PacketCreate(&pack, SERVER_VOTE_TIME_SYNC);
		PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
		server_broadcast(server, &pack, true);
	}

	server->lobby.countdown -= server->delta;
	
	return res;
}

bool mapvote_init(Server* server)
{
	Debug("Attepting to enter ST_MAPVOTE...");
	RAssert(server);

	// randomize
	time_t seed = time(NULL);
	Debug("Mapvote seed: %d", seed);
	srand((unsigned int)seed);

	RAssert(server);
	server->state = ST_MAPVOTE;
	server->lobby.countdown_sec = 30;
	server->lobby.countdown = TICKSPERSEC;
	memset(server->lobby.votes, 0, sizeof(server->lobby.votes));

	MutexLock(g_config.map_list_lock);
	{
		// first we check if map count is less than 3
		int8_t allowed[MAP_COUNT] = { 0 };
		int8_t allowed_count = 0;

		for (int8_t i = 0; i < MAP_COUNT; i++)
		{
			if (g_config.map_list[i])
				allowed[allowed_count++] = i;
		}

		// skip right away if we only have 3 maps
		if (allowed_count <= 3)
		{
			for (int8_t i = 0; i < allowed_count; i++)
			{
				for(int j = i; j < 3; j++)
					server->lobby.maps[j] = allowed[i];
			}
			goto skip_rand;
		}

		int8_t map;
		for (int i = 0; i < 3; i++)
		{
		gen:
			map = allowed[rand() % allowed_count];

			if (map == server->last_map)
				goto gen;

			int num = rand() % 255;
			if (num >= server->map_pickrates[map])
			{
				Debug("%d vs %d lost", num, server->map_pickrates[map]);
				goto gen;
			}

			for (int j = 0; j < i; j++)
			{
				if (server->lobby.maps[j] == map)
					goto gen;
			}

			server->lobby.maps[i] = map;
		}

	}
skip_rand:
	MutexUnlock(g_config.map_list_lock);

	Packet pack;
	PacketCreate(&pack, SERVER_VOTE_MAPS);
	for (int i = 0; i < 3; i++)
		PacketWrite(&pack, packet_write8, server->lobby.maps[i]);
	server_broadcast(server, &pack, true);

	PacketCreate(&pack, SERVER_VOTE_TIME_SYNC);
	PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
	server_broadcast(server, &pack, true);

	Info(LOG_YLW "Server is now in " LOG_PUR "Map Vote");
	Info("Maps: " LOG_RED "[%s] " LOG_BLU "[%s] " LOG_YLW "[%s]", g_mapList[server->lobby.maps[0]].name, g_mapList[server->lobby.maps[1]].name, g_mapList[server->lobby.maps[2]].name);
	return true;
}

bool mapvote_state_join(PeerData* v)
{
	return true;
}

bool mapvote_state_left(PeerData* v)
{
	if(server_ingame(v->server) <= 1)
		return lobby_init(v->server);

	RAssert(mapvote_check_state(v->server));
	return true;
}