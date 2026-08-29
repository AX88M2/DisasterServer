#include "Lib.h"
#include <enet/enet.h>
#include <Server.h>
#include <Config.h>
#include <Colors.h>
#include <CMath.h>
#include <Log.h>
#include <States.h>
#include <Packet.h>
#include <ctype.h>
#include <io/Threads.h>
#include <io/Time.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <cJSON.h>

#ifdef SYS_USE_SDL2
#include <ui/Main.h>
#endif

cJSON *ip_addr_list = NULL;
Mutex ip_addr_mut;

bool peer_identity_process(PeerData *v, const char *addr, bool is_banned, uint64_t timeout, bool do_timeout)
{
	MutexLock(ip_addr_mut);
	{
		if (!v->op && (cJSON_HasObjectItem(ip_addr_list, addr) || cJSON_HasObjectItem(ip_addr_list, v->udid.value)))
		{
			server_disconnect(v->server, v->peer, DR_IPINUSE, NULL);
			MutexUnlock(ip_addr_mut);
			return false;
		}
	}
	MutexUnlock(ip_addr_mut);

	if (is_banned)
	{
		Info("%s banned by host (id %d, ip %s)", v->nickname.value, v->id, addr);
		RAssert(server_disconnect(v->server, v->peer, DR_BANNEDBYHOST, NULL));
		return false;
	}

	if (v->server->peers.noitems >= 7)
	{
		v->should_timeout = false;
		server_disconnect(v->server, v->peer, DR_LOBBYFULL, NULL);
		return false;
	}

	if (do_timeout && timeout != 0)
	{
		time_t tm = time(NULL);
		time_t val = timeout - tm;
		if (val > 0)
		{
			Info("%s is rate-limited (id %d, ip %s)", v->nickname.value, v->id, addr);
			RAssert(server_disconnect(v->server, v->peer, DR_RATELIMITED, NULL));
			return false;
		}
		else
			AssertOrDisconnect(v->server, timeout_revoke(v->udid.value, addr));
	}

	if (!dylist_push(&v->server->peers, v))
	{
		v->should_timeout = false;
		server_disconnect(v->server, v->peer, DR_OTHER, "Report this to dev: code BALLS");
		return false;
	}

	if (!server_state_joined(v))
	{
		v->should_timeout = false;
		server_disconnect(v->server, v->peer, DR_OTHER, "Report this to dev: code WHAR");
		return false;
	}

	// If all checks out send new packet
	Packet pack;
	PacketCreate(&pack, SERVER_IDENTITY_RESPONSE);
	PacketWrite(&pack, packet_write8, v->server->state == ST_LOBBY);
	PacketWrite(&pack, packet_write16, v->id);
	RAssert(packet_send(v->peer, &pack, true));

	// If in queue, do following
	if (!v->in_game)
	{
		// For icons
		for (size_t i = 0; i < v->server->peers.capacity; i++)
		{
			PeerData *peer = (PeerData *)v->server->peers.ptr[i];
			if (!peer)
				continue;

			if (peer->id == v->id)
				continue;

			PacketCreate(&pack, SERVER_WAITING_PLAYER_INFO);
			PacketWrite(&pack, packet_write8, v->server->state == ST_GAME && peer->in_game);
			PacketWrite(&pack, packet_write16, peer->id);
			PacketWrite(&pack, packet_writestr, peer->nickname);

			if (v->server->state == ST_GAME && peer->in_game)
			{
				PacketWrite(&pack, packet_write8, v->server->game.exe == peer->id);
				PacketWrite(&pack, packet_write8, v->server->game.exe == peer->id ? peer->exe_char : peer->surv_char);
			}
			else
			{
				PacketWrite(&pack, packet_write8, peer->lobby_icon);
			}

			RAssert(packet_send(v->peer, &pack, true));
		}

		// For other players in queue
		PacketCreate(&pack, SERVER_WAITING_PLAYER_INFO);
		PacketWrite(&pack, packet_write8, 0);
		PacketWrite(&pack, packet_write16, v->id);
		PacketWrite(&pack, packet_writestr, v->nickname);
		PacketWrite(&pack, packet_write8, v->lobby_icon);
		RAssert(server_broadcast_ex(v->server, &pack, true, v->id));

		char msg[100];
		snprintf(msg, 100, "server " CLRCODE_RED "%d" CLRCODE_RST " of " CLRCODE_BLU "%d" CLRCODE_RST, v->server->id + 1, g_config.server_count);

		server_send_msg(v->server, v->peer, "-----------------------");
		server_send_msg(v->server, v->peer, CLRCODE_RED "better/server~ v" STRINGIFY(BUILD_VERSION));
		server_send_msg(v->server, v->peer, "build from " CLRCODE_PUR __DATE__ " " CLRCODE_GRN __TIME__ CLRCODE_RST);
		server_send_msg(v->server, v->peer, msg);
		server_send_msg(v->server, v->peer, "-----------------------");
		server_send_msg(v->server, v->peer, g_config.motd);

		if (v->op)
			server_send_msg(v->server, v->peer, CLRCODE_GRN "you're an operator on this server" CLRCODE_RST);

		if (v->server->state >= ST_GAME)
		{
			snprintf(msg, 100, "map: " CLRCODE_GRN "%s" CLRCODE_RST, g_mapList[v->server->game.map].name);
			server_send_msg(v->server, v->peer, msg);
		}
	}

	MutexLock(ip_addr_mut);
	cJSON_AddItemToObject(ip_addr_list, addr, cJSON_CreateTrue());
	cJSON_AddItemToObject(ip_addr_list, v->udid.value, cJSON_CreateTrue());
	MutexUnlock(ip_addr_mut);
	return true;
}

bool peer_identity(PeerData *v, Packet *packet)
{
	RAssert(v->id > 0);
	srand((unsigned int)time(NULL));

	bool is_banned;
	uint64_t timeout;

	// Read header
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);
	PacketRead(build_version, packet, packet_read16, uint16_t);
	PacketRead(server_index, packet, packet_read32, int32_t);
	PacketRead(nickname, packet, packet_readstr, String);
	PacketRead(udid, packet, packet_readstr, String);
	PacketRead(lobby_icon, packet, packet_read8, uint8_t);
	PacketRead(pet, packet, packet_read8, int8_t);

	RAssert(ban_check(udid.value, v->ip.value, &is_banned));
	RAssert(timeout_check(udid.value, v->ip.value, &timeout));
	RAssert(op_check(v->ip.value, &v->op));

	v->should_timeout = true;
	v->disconnecting = false;
	v->mod_tool = false;
	v->is_mobile = false;
	v->nickname = nickname;
	v->udid = udid;
	v->lobby_icon = lobby_icon;
	v->pet = pet;

	if (g_config.anticheat)
	{
		AssertOrDisconnect(v->server, auth_verify_ticket(v, packet));
	}

	bool res = true;
	MutexLock(v->server->state_lock);
	{
		v->in_game = (v->server->state == ST_LOBBY);
		v->exe_chance = 1 + rand() % 4;

		if (v->server->peers.noitems >= 7)
		{
			for (int i = 0; i < disaster_count(); i++)
			{
				Server *server = disaster_get(i);
				if (!server)
					continue;

				if (server->peers.noitems >= 7)
					continue;

				Packet pack;
				PacketCreate(&pack, SERVER_LOBBY_CHANGELOBBY);
				PacketWrite(&pack, packet_write32, g_config.port + server->id);
				packet_send(v->peer, &pack, true);

				Debug("Redirecting %d to another free server: %d", v->id, server->id);
				res = false;
				goto quit;
			}

			server_disconnect(v->server, v->peer, DR_LOBBYFULL, NULL);
			res = false;
			goto quit;
		}

		if (type != IDENTITY)
		{
			server_disconnect(v->server, v->peer, DR_OTHER, "type != IDENTITY?");
			res = false;
			goto quit;
		}

		if (passtrough)
		{
			server_disconnect(v->server, v->peer, DR_OTHER, "passtrough?");
			res = false;
			goto quit;
		}

		if (build_version != BUILD_VERSION)
		{
			server_disconnect(v->server, v->peer, DR_VERMISMATCH, NULL);
			res = false;
			goto quit;
		}

		if (string_length(&nickname) >= 30)
		{
			server_disconnect(v->server, v->peer, DR_OTHER, "Your nickname is too long! (30 characters max)");
			res = false;
			goto quit;
		}

		if (udid.len <= 0)
		{
			server_disconnect(v->server, v->peer, DR_OTHER, "whoops you have to put the CD in you conputer");
			res = false;
			goto quit;
		}

		res = peer_identity_process(v, v->ip.value, is_banned, timeout, server_index == -1);
		if (!res)
			goto quit;

		Info("%s (id %d) " LOG_YLW "joined.", nickname.value, v->id);
		Info("	IP: %s", v->ip.value);
		Info("	UID: %s", udid.value);
		Info("	Modified: %s", BoolStringify(v->mod_tool));
		Info("	Mobile: %s", BoolStringify(v->is_mobile));
		v->verified = true;
	}

quit:
	MutexUnlock(v->server->state_lock);
	return res;
}

bool peer_msg(PeerData *v, Packet *packet)
{
	if (v->id == 0)
		return false;

	bool res;
	MutexLock(v->server->state_lock);
	{
		res = server_state_handle(v, packet);
	}
	MutexUnlock(v->server->state_lock);

	return res;
}

bool server_worker(Server *server)
{
	srand((unsigned int)time(NULL));

	char thread_name[128];
	snprintf(thread_name, 128, "Worker Thr %d", server->id);
	ThreadVarSet(g_threadName, thread_name);

	if (!ip_addr_list)
	{
		ip_addr_list = cJSON_CreateObject();
		RAssert(ip_addr_list);
		MutexCreate(ip_addr_mut);
	}

	TimeStamp ticker;
	time_start(&ticker);

	double next_tick = time_end(&ticker);
	double heartbeat = 0.0;
	const double TARGET_FPS = 1000.0 / 60;

	Packet pack;
	PacketCreate(&pack, SERVER_HEARTBEAT);

	while (server->running)
	{
		ENetEvent ev;
		if (enet_host_service(server->host, &ev, 5) > 0)
		{
			switch (ev.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
			{
				Debug("ENET_EVENT_TYPE_CONNECT...");
				ev.peer->data = (PeerData *)malloc(sizeof(PeerData));
				if (!ev.peer->data)
					return false;

				memset(ev.peer->data, 0, sizeof(PeerData));

				PeerData *v = (PeerData *)ev.peer->data;
				v->server = server;
				v->peer = ev.peer;
				v->id = ev.peer->incomingPeerID + 1;
				enet_address_get_host_ip(&ev.peer->address, v->ip.value, 250);

				Packet packet;
				PacketCreate(&packet, SERVER_PREIDENTITY);
				RAssert(auth_create_ticket(v, &packet));
				RAssert(packet_send(ev.peer, &packet, true));
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:
			{
				Debug("ENET_EVENT_TYPE_DISCONNECT...");
				PeerData *v = (PeerData *)ev.peer->data;
				if (!v)
					break;

				if (!v->op && v->should_timeout)
				{
					uint64_t result;
					if (timeout_check(v->udid.value, v->ip.value, &result) && result == 0)
						timeout_set(v->nickname.value, v->udid.value, v->ip.value, time(NULL) + 5);
				}

				if (v->verified)
				{
					MutexLock(ip_addr_mut);
					{
						cJSON_DeleteItemFromObject(ip_addr_list, v->udid.value);
						cJSON_DeleteItemFromObject(ip_addr_list, v->ip.value);
					}
					MutexUnlock(ip_addr_mut);

					MutexLock(v->server->state_lock);
					{
						// Step 3: Cleanup (Only if joined before)
						if (dylist_remove(&v->server->peers, v))
							server_state_left(v);
					}
					MutexUnlock(v->server->state_lock);
				}

				Info("%s (id %d) " LOG_YLW "left.", v->nickname.value, v->id);
				free(v);
				break;
			}

			case ENET_EVENT_TYPE_RECEIVE:
			{
				PeerData *v = (PeerData *)ev.peer->data;
				Packet packet = packet_from(ev.packet);

				switch (packet.buff[1])
				{
				case IDENTITY:
				{
					if (!peer_identity(v, &packet))
					{
						Debug("Identity failed for id %d", v->id);
					}
					break;
				}
				default:
				{
					if (!peer_msg(v, &packet))
						break;
				}
				}

				break;
			}
			}
		}

		double now = time_end(&ticker);
		while (next_tick < now)
		{
			next_tick += TARGET_FPS;
			MutexLock(server->state_lock);
			{
				switch (server->state)
				{
				case ST_LOBBY:
				case ST_CHARSELECT:
				case ST_MAPVOTE:
					lobby_state_tick(server);
					break;

				case ST_GAME:
					game_state_tick(server);
					break;

				case ST_RESULTS:
					results_state_tick(server);
					break;
				}

				// Heartbeat
				if (server->peers.noitems > 0)
				{
					server_broadcast(server, &pack, true);
					if (heartbeat >= (TICKSPERSEC * 2))
					{
						Debug("Heartbeat done.");
						heartbeat = 0;
					}
					heartbeat += server->delta;
				}
			}
			MutexUnlock(server->state_lock);
			server->delta = 1;
		}
	}

	enet_host_destroy(server->host);
	return true;
}

bool server_disconnect(Server *server, ENetPeer *peer, DisconnectReason reason, const char *text)
{
	if (server)
	{
		PeerData *data = (PeerData *)peer->data;
		if (data->disconnecting)
			return true;

		// FIXME: crashes v110 too lazy to fix
		// if(reason == DR_OTHER && text != NULL)
		// {
		// 	Packet pack;
		// 	PacketCreate(&pack, SERVER_PLAYER_FORCE_DISCONNECT);
		// 	PacketWrite(&pack, packet_write8, reason);
		// 	PacketWrite(&pack, packet_writestr, __Str(text));
		// 	packet_send(peer, &pack, true);
		// 	enet_peer_disconnect_later(peer, reason);
		// }
		// else
		enet_peer_disconnect(peer, reason);

		if (!text)
		{
			Info("Disconnected id %d %d: No text.", data->id, reason);
		}
		else
		{
			Info("Disconnected id %d %d: %s.", data->id, reason, text);
		}

		data->disconnecting = true;
	}
	else
		return false;

	return true;
}

bool server_disconnect_id(Server *server, uint16_t id, DisconnectReason reason, const char *text)
{
	if (server)
	{
		bool found = false;
		for (size_t i = 0; i < server->peers.capacity; i++)
		{
			PeerData *data = server->peers.ptr[i];
			if (!data)
				continue;

			if (data->id == id)
				return server_disconnect(server, data->peer, reason, text);
		}
	}
	else
		return false;

	return true;
}

int server_total(Server *server)
{
	int count = 0;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *peer = (PeerData *)server->peers.ptr[i];
		if (!peer)
			continue;

		count++;
	}

	return count;
}

int server_ingame(Server *server)
{
	int count = 0;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *peer = (PeerData *)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->in_game)
			count++;
	}

	return count;
}

PeerData *server_find_peer(Server *server, uint16_t id)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *v = (PeerData *)server->peers.ptr[i];
		if (!v)
			continue;

		if (v->id == id)
			return v;
	}

	return NULL;
}

bool server_broadcast(Server *server, Packet *packet, bool reliable)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *v = (PeerData *)server->peers.ptr[i];
		if (!v)
			continue;

		if (!packet_send(v->peer, packet, reliable))
			server_disconnect(server, v->peer, DR_SERVERTIMEOUT, NULL);
	}

	return true;
}

bool server_broadcast_ex(Server *server, Packet *packet, bool reliable, uint16_t ignore)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData *v = (PeerData *)server->peers.ptr[i];
		if (!v)
			continue;

		if (v->id == ignore)
			continue;

		if (!packet_send(v->peer, packet, reliable))
			server_disconnect(server, v->peer, DR_SERVERTIMEOUT, NULL);
	}

	return true;
}

bool server_state_joined(PeerData *v)
{
#ifdef SYS_USE_SDL2
	ui_update_playerlist(v->server);
#endif

	Packet pack;
	PacketCreate(&pack, SERVER_LOBBY_EXE_CHANCE);
	PacketWrite(&pack, packet_write8, v->exe_chance);
	RAssert(packet_send(v->peer, &pack, true));

	PacketCreate(&pack, SERVER_PLAYER_JOINED);
	PacketWrite(&pack, packet_write16, v->id);
	PacketWrite(&pack, packet_writestr, v->nickname);
	PacketWrite(&pack, packet_write8, v->lobby_icon);
	PacketWrite(&pack, packet_write8, v->pet);
	server_broadcast_ex(v->server, &pack, true, v->id);

	switch (v->server->state)
	{
	case ST_LOBBY:
	case ST_CHARSELECT:
	case ST_MAPVOTE:
		return lobby_state_join(v);

	case ST_GAME:
		return game_state_join(v);

	case ST_RESULTS:
		break;
	}

	return true;
}

bool server_state_handle(PeerData *v, Packet *packet)
{
	switch (v->server->state)
	{
	case ST_LOBBY:
	case ST_CHARSELECT:
	case ST_MAPVOTE:
		return lobby_state_handle(v, packet);

	case ST_GAME:
		return game_state_handletcp(v, packet);

	case ST_RESULTS:
		return results_state_handle(v, packet);
	}

	return true;
}

bool server_state_left(PeerData *v)
{
#ifdef SYS_USE_SDL2
	ui_update_playerlist(v->server);
#endif

	Packet pack;
	PacketCreate(&pack, SERVER_PLAYER_LEFT);
	PacketWrite(&pack, packet_write16, v->id);
	server_broadcast(v->server, &pack, true);

	switch (v->server->state)
	{
	case ST_LOBBY:
	case ST_CHARSELECT:
	case ST_MAPVOTE:
		return lobby_state_left(v);

	case ST_GAME:
		return game_state_left(v);

	case ST_RESULTS:
		break;
	}

	return true;
}

unsigned long server_cmd_parse(String *string)
{
	static const char *clr_list[] = CLRLIST;

	String current = {.len = 0};
	bool found_digit = false;

	for (int i = 0; i < string->len; i++)
	{
		if (!found_digit && isspace(string->value[i]))
			continue;
		else
			found_digit = true;

		if (isspace(string->value[i]))
			break;

		bool invalid = false;
		for (int j = 0; j < CLRLIST_LEN; j++)
		{
			if (string->value[i] == clr_list[j][0])
			{
				invalid = true;
				break;
			}
		}

		if (invalid)
			continue;

		current.value[current.len++] = string->value[i];
	}
	current.value[current.len++] = '\0';

	unsigned int hash = 0;
	for (int i = 0; current.value[i] != '\0'; i++)
		hash = 31 * hash + current.value[i];

	return hash;
}

bool server_cmd_handle(Server *server, unsigned long hash, PeerData *v, String *msg)
{
	Packet pack;
	switch (hash)
	{
	default:
		return false;

	case CMD_BAN:
	{
		if (!v->op)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "you aren't an operator."));
			break;
		}

		int ingame = server_total(v->server);
		if (ingame <= 1)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "dude are you gonna ban yourself?"));
			break;
		}

		PacketCreate(&pack, SERVER_LOBBY_CHOOSEBAN);
		RAssert(packet_send(v->peer, &pack, true));
		break;
	}

	case CMD_KICK:
	{
		if (!v->op)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "you aren't an operator."));
			break;
		}

		if (server_total(v->server) <= 1)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "dude are you gonna kick yourself?"));
			break;
		}

		PacketCreate(&pack, SERVER_LOBBY_CHOOSEKICK);
		RAssert(packet_send(v->peer, &pack, true));
		break;
	}

	case CMD_OP:
	{
		if (!v->op)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "you aren't an operator."));
			break;
		}

		if (server_total(v->server) <= 1)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "you're already an operator tho??"));
			break;
		}

		PacketCreate(&pack, SERVER_LOBBY_CHOOSEOP);
		RAssert(packet_send(v->peer, &pack, true));
		break;
	}

	case CMD_LOBBY:
	{
		int ind;
		if (sscanf(msg->value, ".lobby %d", &ind) <= 0)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "example:~ .lobby 1"));
			break;
		}

		if (ind < 1 || ind > disaster_count())
		{
			char msg[128];
			snprintf(msg, 128, CLRCODE_RED "lobby should be between 1 and %d", disaster_count());
			RAssert(server_send_msg(v->server, v->peer, msg));
			break;
		}

		PacketCreate(&pack, SERVER_LOBBY_CHANGELOBBY);
		PacketWrite(&pack, packet_write32, g_config.port + ind - 1);
		RAssert(packet_send(v->peer, &pack, true));
		break;
	}

	/* Help message  */
	case CMD_HELP:
	{
		char lobby_msg[128];
		snprintf(lobby_msg, 128, CLRCODE_GRA ".lobby" CLRCODE_RST " choose lobby (1-%d)", disaster_count());

		RAssert(server_send_msg(v->server, v->peer, CLRCODE_GRA ".info" CLRCODE_RST " server info"));
		RAssert(server_send_msg(v->server, v->peer, lobby_msg));
		RAssert(server_send_msg(v->server, v->peer, CLRCODE_GRA ".vk" CLRCODE_RST " vote kick"));
		RAssert(server_send_msg(v->server, v->peer, CLRCODE_GRA ".vp" CLRCODE_RST " vote practice mode"));

		if (v->op)
		{
			snprintf(lobby_msg, 128, CLRCODE_GRA CLRCODE_GRA ".map" CLRCODE_RST " choose map (1-%d)", MAP_COUNT + 1);
			RAssert(server_send_msg(v->server, v->peer, lobby_msg));

			RAssert(server_send_msg(v->server, v->peer, CLRCODE_GRA ".kick" CLRCODE_RST " kick someone ig"));
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_GRA ".ban" CLRCODE_RST " ban someone ig"));
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_GRA ".op" CLRCODE_RST " op someone ig"));
		}

		break;
	}

	/* Information about the lobby */
	case CMD_INFO:
	{
		char msg[100];
		snprintf(msg, 100, "server " CLRCODE_RED "%d" CLRCODE_RST " of " CLRCODE_BLU "%d" CLRCODE_RST, v->server->id + 1, g_config.server_count);

		server_send_msg(v->server, v->peer, "-----------------------");
		server_send_msg(v->server, v->peer, CLRCODE_RED "better" CLRCODE_BLU "server" CLRCODE_RST " v" STRINGIFY(BUILD_VERSION));
		server_send_msg(v->server, v->peer, "build from " CLRCODE_PUR __DATE__ " " CLRCODE_GRN __TIME__ CLRCODE_RST);
		server_send_msg(v->server, v->peer, msg);
		server_send_msg(v->server, v->peer, "-----------------------");
		server_send_msg(v->server, v->peer, CLRCODE_GRA "type .help for command list" CLRCODE_RST);
		server_send_msg(v->server, v->peer, g_config.motd);
		break;
	}

	/* Does he know? */
	case CMD_STINK:
	{
		char buff[36];
		if (sscanf(msg->value, ".stink %35s", buff) <= 0)
		{
			RAssert(server_send_msg(v->server, v->peer, CLRCODE_RED "example:~ .stink baller"));
			break;
		}

		char format[100];
		snprintf(format, 100, "\\%s~, you /sti@nk~", buff);

		RAssert(server_broadcast_msg(v->server, format));
		break;
	}
	}

	return true;
}

bool server_msg_handle(Server *server, PacketType type, PeerData *v, Packet *packet)
{
	switch (type)
	{
	default:
		break;

	case CLIENT_LOBBY_CHOOSEBAN:
	{
		if (!v->op)
			break;

		PacketRead(pid, packet, packet_read16, uint16_t);

		for (size_t i = 0; i < v->server->peers.capacity; i++)
		{
			PeerData *peer = (PeerData *)v->server->peers.ptr[i];
			if (!peer)
				continue;

			if (peer->id == pid)
			{
				RAssert(ban_add(peer->nickname.value, peer->udid.value, peer->ip.value));
				server_disconnect(v->server, peer->peer, DR_BANNEDBYHOST, NULL);
				break;
			}
		}
		break;
	}

	case CLIENT_LOBBY_CHOOSEKICK:
	{
		if (!v->op)
			break;

		PacketRead(pid, packet, packet_read16, uint16_t);

		for (size_t i = 0; i < v->server->peers.capacity; i++)
		{
			PeerData *peer = (PeerData *)v->server->peers.ptr[i];
			if (!peer)
				continue;

			if (peer->id == pid)
			{
				RAssert(timeout_set(peer->nickname.value, peer->udid.value, peer->ip.value, time(NULL) + 60));
				server_disconnect(v->server, peer->peer, DR_KICKEDBYHOST, NULL);
				break;
			}
		}
		break;
	}

	case CLIENT_LOBBY_CHOOSEOP:
	{
		if (!v->op)
			break;

		PacketRead(pid, packet, packet_read16, uint16_t);

		for (size_t i = 0; i < v->server->peers.capacity; i++)
		{
			PeerData *peer = (PeerData *)v->server->peers.ptr[i];
			if (!peer)
				continue;

			if (peer->id == pid)
			{
				peer->op = true;

				RAssert(op_add(peer->nickname.value, peer->ip.value));
				server_send_msg(v->server, peer->peer, CLRCODE_GRN "you're an operator now");
				break;
			}
		}
		break;
	}
	}

	return true;
}

bool server_send_msg(Server *server, ENetPeer *peer, const char *message)
{
	Packet pack;
	PacketCreate(&pack, CLIENT_CHAT_MESSAGE);
	PacketWrite(&pack, packet_write16, 0);
	PacketWrite(&pack, packet_writestr, string_lower(__Str(message)));

	RAssert(packet_send(peer, &pack, true));
	return true;
}

bool server_broadcast_msg(Server *server, const char *message)
{
	Packet pack;
	PacketCreate(&pack, CLIENT_CHAT_MESSAGE);
	PacketWrite(&pack, packet_write16, 0);
	PacketWrite(&pack, packet_writestr, string_lower(__Str(message)));

	server_broadcast(server, &pack, true);
	return true;
}
