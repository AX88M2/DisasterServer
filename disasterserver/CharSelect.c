#include <Log.h>
#include <States.h>
#include <CMath.h>
#include <Colors.h>

bool charselect_check_state(Server *server)
{
	RAssert(server);

	bool should = true;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *peer = (PeerData *)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!peer->in_game)
			continue;

		if (peer->exe_char == CH_NONE && peer->surv_char == EX_NONE)
			should = false;
	}

	if (should)
		return game_init(server->lobby.exe, server->lobby.map, server);

	return true;
}

bool charselect_choose_exe(Server *server, uint16_t *id)
{
	RAssert(server);

	uint32_t weight = 0;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *peer = (PeerData *)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!peer->in_game)
			continue;

		// Reset all characters
		peer->exe_char = EX_NONE;
		peer->surv_char = CH_NONE;

		weight += peer->exe_chance;
	}

	if (weight == 0)
		weight++;

	uint32_t rnd = rand() % weight;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *peer = (PeerData *)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!peer->in_game)
			continue;

		if (peer->exe_chance >= 100)
		{
			*id = peer->id;
			return true;
		}

		if (rnd < peer->exe_chance && !peer->mod_tool)
		{
			Info("%s (id %d, c %d) is exe!", peer->nickname.value, peer->id, peer->exe_chance);

			peer->exe_chance = 1 + rand() % 1;
			*id = peer->id;

			return true;
		}

		rnd -= peer->exe_chance;
	}

	*id = -1;
	return false;
}

bool charselect_state_handle(PeerData *v, Packet *packet)
{
	// Read header
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	bool res = true;
	Packet pack;
	switch (type)
	{
	case CLIENT_REQUEST_EXECHARACTER:
	{
		if (!v->in_game)
			break;

		// Sanity check
		AssertOrDisconnect(v->server, v->server->lobby.exe == v->id);
		PacketRead(id, packet, packet_read8, uint8_t);

		id--; // id - 1
		v->exe_char = id;

		AssertOrDisconnect(v->server, id >= 0);
		AssertOrDisconnect(v->server, id <= EX_EXELLER);

		PacketCreate(&pack, SERVER_LOBBY_EXECHARACTER_RESPONSE);
		PacketWrite(&pack, packet_write8, id);
		RAssert(packet_send(v->peer, &pack, true));

		PacketCreate(&pack, SERVER_LOBBY_CHARACTER_CHANGE);
		PacketWrite(&pack, packet_write16, v->id);
		PacketWrite(&pack, packet_write8, id);
		server_broadcast(v->server, &pack, true);

		const char *exes[] = {
			"Classic Exe",
			"Chaos",
			"Exetior",
			"Exeller"};

		Info("%s " LOG_RST "(id %d) choses [" LOG_RED "%s" LOG_RST "]!", v->nickname.value, v->id, exes[id]);
		return charselect_check_state(v->server) || lobby_init(v->server);
	}

	case CLIENT_REQUEST_CHARACTER:
	{
		if (!v->in_game)
			break;

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
		PacketWrite(&pack, packet_write8, id + 1);
		PacketWrite(&pack, packet_write8, avail);
		RAssert(packet_send(v->peer, &pack, true));

		if (avail)
		{
			v->surv_char = id;

			PacketCreate(&pack, SERVER_LOBBY_CHARACTER_CHANGE);
			PacketWrite(&pack, packet_write16, v->id);
			PacketWrite(&pack, packet_write8, id + 1);
			server_broadcast(v->server, &pack, true);
		}

		const char *survs[] = {
			"Tails",
			"Knuckles",
			"Eggman",
			"Amy",
			"Cream",
			"Sally"};

		Info("%s " LOG_RST "(id %d) choses [" LOG_GRN "%s" LOG_RST "]!", v->nickname.value, v->id, survs[id]);
		return charselect_check_state(v->server) || lobby_init(v->server);
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

bool charselect_state_tick(Server *server)
{
	if (server->lobby.countdown <= 0)
	{
		server->lobby.countdown += TICKSPERSEC;

		if (--server->lobby.countdown_sec == 0)
		{
			for (size_t i = 0; i < server->peers.capacity; i++)
			{
				PeerData *peer = (PeerData *)server->peers.ptr[i];
				if (!peer)
					continue;

				if (!peer->in_game)
					continue;

				if (peer->exe_char == EX_NONE && peer->surv_char == CH_NONE)
					server_disconnect(server, peer->peer, DR_AFKTIMEOUT, NULL);
			}
		}

		Packet pack;
		PacketCreate(&pack, SERVER_CHAR_TIME_SYNC);
		PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
		server_broadcast(server, &pack, true);
	}

	server->lobby.countdown -= server->delta;
	return true;
}

bool charselect_init(int8_t map, Server *server)
{
	Debug("Attepting to enter ST_CHARSELECT...");
	RAssert(server);
	RAssert(charselect_choose_exe(server, &server->lobby.exe));

	if (server->lobby.exe == -1)
	{
		Err("Failed to pick exe for some reason!");
		return lobby_init(server);
	}

	server->state = ST_CHARSELECT;
	server->lobby.countdown_sec = 30;
	server->lobby.countdown = TICKSPERSEC;
	memset(server->lobby.avail, 1, sizeof(uint8_t) * (CH_SALLY + 1));

	Packet pack;
	PacketCreate(&pack, SERVER_LOBBY_EXE);
	PacketWrite(&pack, packet_write16, server->lobby.exe);
	PacketWrite(&pack, packet_write16, map);
	server_broadcast(server, &pack, true);

	PacketCreate(&pack, SERVER_CHAR_TIME_SYNC);
	PacketWrite(&pack, packet_write8, server->lobby.countdown_sec);
	server_broadcast(server, &pack, true);

	Info(LOG_YLW "Server is now in " LOG_PUR "Character Select");
	server->lobby.map = map;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *data = (PeerData *)server->peers.ptr[i];

		if (!data)
			continue;

		if (data->in_game)
			continue;

		char msg[256];
		snprintf(msg, 100, "map: %s" CLRCODE_RST, g_mapList[map].name);
		server_send_msg(server, data->peer, msg);
	}

	return true;
}

bool charselect_state_join(PeerData *v)
{
	return true;
}

bool charselect_state_left(PeerData *v)
{
	if (v->surv_char != CH_NONE)
		v->server->lobby.avail[v->surv_char] = true;

	if (server_ingame(v->server) <= 1 || v->id == v->server->lobby.exe)
		return lobby_init(v->server);

	return charselect_check_state(v->server) || lobby_init(v->server);
}
