#include <States.h>
#include <CMath.h>
#include <Colors.h>

bool charselect_check_state(Server* server)
{
	uint8_t should = 1;

	RAssert(server);
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!peer->in_game)
			continue;

		if (peer->exe_char == CH_NONE && peer->surv_char == EX_NONE)
			should = 0;
	}
	
	if (should)
		RAssert(game_init(server->lobby.exe, server->lobby.map, server));

	return true;
}

bool charselect_choose_exe(Server* server, int* id)
{
	RAssert(server);
	
	bool res = false;
	uint32_t weight = 0;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!peer->in_game)
			continue;

		// Reset all characters
		peer->exe_char = EX_NONE;
		peer->surv_char = CH_NONE;

		weight += peer->exe_chance;
	}

	uint32_t rnd = rand() % weight;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!peer->in_game)
			continue;

		if(peer->exe_chance >= 100)
		{
			*id = peer->id;
			return true;
		}
		
		if (rnd < peer->exe_chance)
		{
			Info("%s (id %d, c %d) is exe!", peer->nickname.value, peer->id, peer->exe_chance);
			
			peer->exe_chance = 1 + rand() % 1;
			*id = peer->id;

			res = true;
			break;
		}

		rnd -= peer->exe_chance;
	}

	return res;
}

bool charselect_state_handle(PeerData* v, Packet* packet)
{
	// Read header
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	bool res = true;
	Packet pack;
	switch (type)
	{
		default:
			RAssert(server_msg_handle(v->server, type, v, packet));
			break;
			
		case CLIENT_REQUEST_EXECHARACTER:
		{
			// Sanity check
			AssertOrDisconnect(v->server, v->server->lobby.exe == v->id);
			PacketRead(id, packet, packet_read8, uint8_t);

			id--; // id - 1
			v->exe_char = id;

			AssertOrDisconnect(v->server, id >= 0);
			AssertOrDisconnect(v->server, id <= EX_EXELLER);

			PacketCreate(&pack, SERVER_LOBBY_EXECHARACTER_RESPONSE);
			PacketWrite(&pack, packet_write8, id);
			RAssert(packet_sendtcp(v->server, v->id, &pack));

			PacketCreate(&pack, SERVER_LOBBY_CHARACTER_CHANGE);
			PacketWrite(&pack, packet_write16, v->id);
			PacketWrite(&pack, packet_write8, id);
			server_broadcast(v->server, &pack);

			const char* exes[] = {
				"Classic Exe",
				"Chaos",
				"Exetior",
				"Exeller"
			};

			Info("%s (id %d) choses [%s]!", v->nickname.value, v->id, exes[id]);
			charselect_check_state(v->server);
			break;
		}

		case CLIENT_REQUEST_CHARACTER:
		{
			if (v->surv_char != CH_NONE)
				break;

			// Sanity check
			AssertOrDisconnect(v->server, v->server->lobby.exe != v->id);
			PacketRead(id, packet, packet_read8, uint8_t);
			id--; // id - 1

			AssertOrDisconnect(v->server, id >= 0);
			AssertOrDisconnect(v->server, id <= CH_SALLY);

			uint8_t avail = v->server->lobby.avail[id];
			if (avail)
				v->server->lobby.avail[id] = 0;

			PacketCreate(&pack, SERVER_LOBBY_CHARACTER_RESPONSE);
			PacketWrite(&pack, packet_write8, id+1);
			PacketWrite(&pack, packet_write8, avail);
			RAssert(packet_sendtcp(v->server, v->id, &pack));

			if (avail)
			{
				v->surv_char = id;

				PacketCreate(&pack, SERVER_LOBBY_CHARACTER_CHANGE);
				PacketWrite(&pack, packet_write16, v->id);
				PacketWrite(&pack, packet_write8, id+1);
				server_broadcast(v->server, &pack);
			}

			const char* survs[] = {
				"Tails",
				"Knuckles",
				"Eggman",
				"Amy",
				"Cream",
				"Sally"
			};

			Info("%s (id %d) choses [%s]!", v->nickname.value, v->id, survs[id]);
			charselect_check_state(v->server);
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

bool charselect_state_tick(Server* server)
{
	if (server->lobby.countdown <= 0)
	{
		server->lobby.countdown += TICKSPERSEC;

		if (--server->lobby.countdown_sec == 0)
		{
			for (size_t i = 0; i < server->peers.capacity; i++)
			{
				PeerData* peer = (PeerData*)server->peers.ptr[i];
				if (!peer)
					continue;

				if (!peer->in_game)
					continue;

				if (peer->exe_char == EX_NONE && peer->surv_char == CH_NONE)
					server_disconnect(server, peer->id, DR_AFKTIMEOUT, NULL);
			}
		}

		Packet pack;
		PacketCreate(&pack, SERVER_CHAR_TIME_SYNC);
		PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
		server_broadcast(server, &pack);
	}

	server->lobby.countdown -= server->delta;
	return true;
}

bool charselect_init(int8_t map, Server* server)
{
	Debug("Attepting to enter ST_CHARSELECT...");
	RAssert(server);
	RAssert(charselect_choose_exe(server, &server->lobby.exe));

	server->state = ST_CHARSELECT;
	server->lobby.countdown_sec = 30;
	server->lobby.countdown = TICKSPERSEC;
	memset(server->lobby.avail, 1, sizeof(uint8_t) * (CH_SALLY + 1));

	Packet pack;
	PacketCreate(&pack, SERVER_LOBBY_EXE);
	PacketWrite(&pack, packet_write16, server->lobby.exe);
	PacketWrite(&pack, packet_write16, map);
	server_broadcast(server, &pack);

	PacketCreate(&pack, SERVER_CHAR_TIME_SYNC);
	PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
	server_broadcast(server, &pack);

	Info("Server is now in ST_CHARSELECT");
	server->lobby.map = map;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		
		if (!peer)
			continue;

		if (peer->in_game)
			continue;

		char msg[256];
		snprintf(msg, 100, "map: " CLRCODE_GRN "%s" CLRCODE_RST, g_mapList[map].name);
		server_send_msg(server, peer->id, msg);
	}

	return true;
}

bool charselect_state_join(PeerData* v)
{
	return true;
}

bool charselect_state_left(PeerData* v)
{
	if (v->surv_char != CH_NONE)
		v->server->lobby.avail[v->surv_char] = 1;

	if (server_ingame(v->server) <= 1 || v->id == v->server->lobby.exe)
		return lobby_init(v->server);

	RAssert(charselect_check_state(v->server));
	return true;
}