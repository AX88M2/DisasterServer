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
		default:
			RAssert(server_msg_handle(v->server, type, v, packet));
			break;
			
		case CLIENT_VOTE_REQUEST:
		{
			PacketRead(map, packet, packet_read8, uint8_t);
			AssertOrDisconnect(v->server, !v->voted);
			AssertOrDisconnect(v->server, map >= 0);
			AssertOrDisconnect(v->server, map < 3);

			v->server->lobby.votes[map]++;
			v->voted = 1;

			PacketCreate(&pack, SERVER_VOTE_SET);
			for (int i = 0; i < 3; i++)
				PacketWrite(&pack, packet_write8, v->server->lobby.votes[i]);

			Debug("%s (id %d) voted for [%s]!", v->nickname.value, v->id, g_mapList[v->server->lobby.maps[map]].name);
			server_broadcast(v->server, &pack);
			mapvote_check_state(v->server);
			break;
		}

		case CLIENT_CHAT_MESSAGE:
		{
			if (v->in_game)
				break;

			PacketRead(pid, packet, packet_read16, uint16_t);
			PacketRead(msg, packet, packet_readstr, String);
			AssertOrDisconnect(v->server, string_length(&msg) <= 40);

			v->timeout = 0;

			Info("[%s] (id %d): %s", v->nickname.value, v->id, msg.value);
			if (!server_cmd_handle(v->server, server_cmd_parse(&msg), v, &msg))
				server_broadcast_ex(v->server, packet, v->id);

			break;
		}
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
			Info("Map is [%s]", g_mapList[won].name);

			if (!charselect_init(won, server))
				return lobby_init(server);
		}

		Packet pack;
		PacketCreate(&pack, SERVER_VOTE_TIME_SYNC);
		PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
		server_broadcast(server, &pack);
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
	srand((uint32_t)seed);

	RAssert(server);
	server->state = ST_MAPVOTE;
	server->lobby.countdown_sec = 30;
	server->lobby.countdown = TICKSPERSEC;
	
	int map;
	for (int i = 0; i < 3; i++)
	{
	gen:
		map = rand() % MAP_COUNT;

		if (map == server->last_map)
			goto gen;

		for (int j = 0; j < i; j++)
		{
			if (server->lobby.maps[j] == map)
				goto gen;
		}

		server->lobby.maps[i] = map;
	}
	memset(server->lobby.votes, 0, 3 * sizeof(uint8_t));

	Packet pack;
	PacketCreate(&pack, SERVER_VOTE_MAPS);
	for (int i = 0; i < 3; i++)
		PacketWrite(&pack, packet_write8, server->lobby.maps[i]);
	server_broadcast(server, &pack);

	PacketCreate(&pack, SERVER_VOTE_TIME_SYNC);
	PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
	server_broadcast(server, &pack);

	Info("Server is now in ST_MAPVOTE");
	Info("Vote maps are [%s] [%s] [%s]", g_mapList[server->lobby.maps[0]].name, g_mapList[server->lobby.maps[1]].name, g_mapList[server->lobby.maps[2]].name);
	return true;
}

bool mapvote_state_join(PeerData* v)
{
	return true;
}

bool mapvote_state_left(PeerData* v)
{
	uint8_t should = 0;
	if(server_ingame(v->server) <= 1)
		should = 1;
	
	if (should)
		return lobby_init(v->server);

	RAssert(mapvote_check_state(v->server));
	return true;
}