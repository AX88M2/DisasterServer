#include <Server.h>
#include <States.h>
#include <io/TcpListener.h>
#include <io/Packet.h>
#include <io/Threads.h>
#include <io/Time.h>
#include <CMath.h>
#include <Colors.h>
#include <time.h>
#include <string.h>

bool peer_identity(PeerData* v, Packet* packet)
{
	RAssert(v->id > 0);
	srand(time(NULL));

	Packet pack;
	bool result = true;

	// Read header
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);
	AssertOrDisconnect(v->server, type == IDENTITY);

	PacketRead(build_version, packet, packet_read16, uint16_t);
	PacketRead(nickname, packet, packet_readstr, String);
	PacketRead(lobby_icon, packet, packet_read8, uint8_t);
	PacketRead(pet, packet, packet_read8, int8_t);
	PacketRead(os_type, packet, packet_read8, uint8_t);
	PacketRead(udid, packet, packet_readstr, String);

	AssertOrDisconnect(v->server, !passtrough);

	if(build_version != BUILD_VERSION)
	{
		server_disconnect(v->server, v->id, DR_VERMISMATCH, NULL);	
		return false;
	}

	AssertOrDisconnect(v->server, string_length(&nickname) < 30);
	AssertOrDisconnect(v->server, udid.len > 0);
	AssertOrDisconnect(v->server, os_type < 3);

	const char* addr = inet_ntoa(v->addr.sin_addr);

	uint8_t is_banned;
	uint64_t timeout;
	AssertOrDisconnect(v->server, ban_check(udid.value, addr, &is_banned));
	AssertOrDisconnect(v->server, timeout_check(udid.value, addr, &timeout));

	v->in_game = v->server->state == ST_LOBBY;
	v->nickname = nickname;
	v->udid = udid;
	v->lobby_icon = lobby_icon;
	v->pet = pet;
	v->exe_chance = 1 + rand() % 4;

	if (is_banned)
	{
		Info("%s banned by host (id %d, ip %s)", nickname.value, v->id, addr);
		RAssert(server_disconnect(v->server, v->id, DR_BANNEDBYHOST, NULL));
		return false;
	}

	if (timeout != 0)
	{
		time_t tm = time(NULL);
		time_t val = timeout - tm;
		if (val > 0)
		{
			Info("%s is rate-limited (id %d, ip %s)", nickname.value, v->id, addr);
			RAssert(server_disconnect(v->server, v->id, DR_RATELIMITED, NULL));
			return false;
		}
		else
			AssertOrDisconnect(v->server, timeout_revoke(v->udid.value, addr));
	}
	
	// set all info or something idk
	if (!op_check(addr, &v->op))
	{
		server_disconnect(v->server, v->id, DR_OTHER, "Check for an operator failed!");
		return false;
	}

	if (v->server->peers.noitems >= 7)
	{
		server_disconnect(v->server, v->id, DR_LOBBYFULL, NULL);
		return false;
	}
	
	if (!dylist_push(&v->server->peers, v))
	{
		server_disconnect(v->server, v->id, DR_OTHER, "Report this to dev: code BALLS");
		return false;
	}

	if (!server_state_joined(v))
	{
		server_disconnect(v->server, v->id, DR_OTHER, "Report this to dev: code WHAR");
		return false;
	}

	// If all checks out send new packet
	PacketCreate(&pack, SERVER_IDENTITY_RESPONSE);
	PacketWrite(&pack, packet_write8, v->server->state == ST_LOBBY);
	PacketWrite(&pack, packet_write16, ntohs(v->server->udp.addr.sin_port));
	PacketWrite(&pack, packet_write16, v->id);
	RAssert(packet_sendtcp(v->server, v->id, &pack));

	// If in queue, do following
	if (!v->in_game)
	{
		// For icons
		for (size_t i = 0; i < v->server->peers.capacity; i++)
		{
			PeerData* peer = (PeerData*)v->server->peers.ptr[i];
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

			RAssert(packet_sendtcp(v->server, v->id, &pack));
		}

		// For other players in queue
		PacketCreate(&pack, SERVER_WAITING_PLAYER_INFO);
		PacketWrite(&pack, packet_write8, 0);
		PacketWrite(&pack, packet_write16, v->id);
		PacketWrite(&pack, packet_writestr, v->nickname);
		PacketWrite(&pack, packet_write8, v->lobby_icon);
		RAssert(server_broadcast_ex(v->server, &pack, v->id));

		char msg[100];
		snprintf(msg, 100, "server " CLRCODE_RED "%d" CLRCODE_RST " of " CLRCODE_BLU "%d" CLRCODE_RST, v->server->id + 1, g_config.server_count);

		server_send_msg(v->server, v->id, "-----------------------");
		server_send_msg(v->server, v->id, CLRCODE_RED "better/server~ v" STRINGIFY(BUILD_VERSION));
		server_send_msg(v->server, v->id, "build from " CLRCODE_PUR  __DATE__ " " CLRCODE_GRN  __TIME__ CLRCODE_RST);
		server_send_msg(v->server, v->id, msg);
		server_send_msg(v->server, v->id, "-----------------------");

		if (v->op)
			server_send_msg(v->server, v->id, CLRCODE_GRN "you're an operator on this server" CLRCODE_RST);
		
		if (v->server->state >= ST_GAME)
		{
			snprintf(msg, 100, "map: " CLRCODE_GRN "%s" CLRCODE_RST, g_mapList[v->server->game.map].name);
			server_send_msg(v->server, v->id, msg);
		}
	}

	char* os_array[] = 
	{
		"Windows",
		"Linux",
		"Android",
		"Mac OS"
	};

	Info("Identity for id %d: ", v->id);
	Info("	Nickname: %s", nickname.value);
	Info("	Operating System: %s", os_array[os_type]);
	Info("	Unique ID: %s", udid.value);

	Info("Player reached stage 4");
	return true;
}

bool peer_msg(PeerData* v, Packet* packet)
{
	if (v->id == 0)
		return false;

	MutexLock(v->server->state_lock);
		bool res = server_state_handle(v, packet);
	MutexUnlock(v->server->state_lock);

	return res;
}

bool server_peerworker(PeerData* v)
{
	Packet packet;

	// Step 1: Identity
	if (!packet_tcprecv(v->id, &packet))
	{
		Err("Step 1 failed for id %d (s1)", v->id);
		goto cleanup;
	}
	
	Info("Player reached stage 0");
	MutexLock(v->server->state_lock);
	if (!peer_identity(v, &packet))
	{
		MutexUnlock(v->server->state_lock);
		Err("Step 1 failed for id %d (s2)", v->id);
		goto cleanup;
	}
	MutexUnlock(v->server->state_lock);

	// Step 2: Listen for packets
	while (1)
	{
		if (!packet_tcprecv(v->id, &packet))
			break;
	
		if (!peer_msg(v, &packet))
			break;
	}

cleanup:
	MutexLock(v->server->state_lock);
	{
		// Step 3: Cleanup (Only if joined before)
		if (dylist_remove(&v->server->peers, v))
			server_state_left(v);
	}
	MutexUnlock(v->server->state_lock);

	if(!v->op)
	{
		// Check if they are in timeouts
		uint64_t result;
		const char* addr = inet_ntoa(v->addr.sin_addr);
	
		if (timeout_check(v->udid.value, addr, &result) && result == 0)
			timeout_set(v->nickname.value, addr, v->udid.value, time(NULL) + 5);
	}
	
	tcp_kill(v->server->tcp, v->id);
	free(v);

	return true;
}

bool server_udpworker(Server* server)
{
	Packet pack;
	IpAddr addr;

	while (1)
	{
		if (!packet_udprecv(server->udp.fd, &pack, &addr))
			continue;

		if (server->state != ST_GAME)
			continue;

		Timer t;
		time_start(&t);
		MutexLock(server->state_lock);
		{
			double elapsed = time_end(&t);
			game_state_handleudp(server, &addr, &pack);

			if (elapsed > 2)
				Warn("it took %f ms to aquire UDP lock, lag spike!", elapsed);
		}
		MutexUnlock(server->state_lock);
	}

	return true;
}

bool server_tick(Server* server)
{
	Timer			start;
	Packet			pack;
	double			heartbeat = 0.0;
	const double	TARGET_FPS = 1000.0 / 60;

	while (1)
	{
		time_start(&start);
		MutexLock(server->state_lock);
		{
			if (server->state < ST_GAME)
				lobby_state_tick(server);
			else
				game_state_tick(server);

			// Heartbeat
			if (server->peers.noitems > 0)
			{
				if (heartbeat >= (TICKSPERSEC * 2))
				{
					PacketCreate(&pack, SERVER_HEARTBEAT);
					server_broadcast(server, &pack);

					Debug("Heartbeat done.");
					heartbeat = 0;
				}

				heartbeat += server->delta;
			}
		}
		MutexUnlock(server->state_lock);

		if (time_end(&start) < 2)
			ThreadSleep(10);

		while (time_end(&start) < 15);
		double elapsed = time_end(&start);

		server->delta = elapsed / TARGET_FPS;
	}
}

bool server_disconnect(Server* server, int id, DisconnectReason reason, const char* text)
{
	bool res = true;

	if (!text)
	{
		Info("Disconnecting id %d with error code %d: No text.", id, reason);
	}
	else
	{
		Info("Disconnecting id %d with error code %d: %s.", id, reason, text);
	}

	uint8_t found = 0;

	// we DONT FUCKING CARE, GET RID OF THEM
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->id == id)
		{
			Debug("Attempt to remove player %d from the ugly list", peer->id);
			if (dylist_remove(&server->peers, peer))
			{
				server_state_left(peer);
				found = 1;
				break;
			}

			Debug("This didnt go well for %d!", peer->id);
			break;
		}
	}

	if (!found && reason == DR_DONTREPORT)
		return true;

	if (reason != DR_DONTREPORT)
	{
		Packet pack;
		PacketCreate(&pack, SERVER_PLAYER_FORCE_DISCONNECT);
		PacketWrite(&pack, packet_write8, reason);

		if (reason == DR_OTHER)
		{
			PacketWrite(&pack, packet_writestr, string_lower(__Str(text)));
		}

		res = packet_sendtcp(server, id, &pack);
	}

#ifdef WIN32
	int how = SD_BOTH;
#else
	int how = SHUT_RDWR;
#endif

	shutdown(id, how);
	return res;
}

bool server_timeout(Server* server, uint16_t id, double timeout)
{
	MutexLock(server->state_lock);
	bool res = false;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->id == id)
			res = timeout_set(peer->nickname.value, inet_ntoa(peer->addr.sin_addr), peer->udid.value, time(NULL) + (uint64_t)(60 * timeout));
	}

	if (res)
		res = server_disconnect(server, id, DR_KICKEDBYHOST, NULL);

	MutexUnlock(server->state_lock);
	return res;
}

bool server_ban(Server* server, uint16_t id)
{
	MutexLock(server->state_lock);
	bool res = false;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->id == id)
			res = ban_add(peer->nickname.value, inet_ntoa(peer->addr.sin_addr), peer->udid.value);
	}

	if (res)
		res = server_disconnect(server, id, DR_BANNEDBYHOST, NULL);

	MutexUnlock(server->state_lock);
	return res;
}

bool server_op(Server* server, uint16_t id)
{
	bool res = false;
	MutexLock(server->state_lock);
	{
		for (size_t i = 0; i < server->peers.capacity; i++)
		{
			PeerData* peer = (PeerData*)server->peers.ptr[i];
			if (!peer)
				continue;

			if (peer->id == id)
			{
				peer->op = 1;
				res = op_add(peer->nickname.value, inet_ntoa(peer->addr.sin_addr));
				break;
			}
		}

		if (res)
			res = server_send_msg(server, id, CLRCODE_GRN "you're an operator now");
	}
	MutexUnlock(server->state_lock);
	return res;
}

bool server_playerinfo(Server* server, int index, PlayerInfo* info)
{
	info->character = -1;

	bool res = true;
	if (index < 0 || index >= server->peers.capacity)
		res = false;
	else
	{
		PeerData* peer = (PeerData*)server->peers.ptr[index];
		if (!peer)
			res = false;
		else
		{
			memcpy(&info->nickname, &peer->nickname, sizeof(String));

			info->id = peer->id;
			info->is_exe = server->game.exe == peer->id;

			if (peer->in_game && server->state >= ST_GAME)
			{
				if (info->is_exe)
					info->character = peer->exe_char;
				else
					info->character = peer->surv_char;
			}
		}
	}

	return res;
}

int server_total(Server* server)
{
	int count = 0;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		count++;
	}

	return count;
}

int server_ingame(Server* server)
{
	int count = 0;
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->in_game)
			count++;
	}

	return count;
}

uint8_t server_state(Server* server)
{
	return (uint8_t)server->state;
}

bool server_broadcast(Server* server, Packet* packet)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!packet_sendtcp(server, peer->id, packet))
			server_disconnect(server, peer->id, DR_TCPTIMEOUT, NULL);
	}

	return true;
}

bool server_broadcast_ex(Server* server, Packet* packet, uint16_t ignore)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->id == ignore)
			continue;

		if (!packet_sendtcp(server, peer->id, packet))
			server_disconnect(server, peer->id, DR_TCPTIMEOUT, NULL);
	}

	return true;
}

bool server_state_joined(PeerData* v)
{
	Packet pack;

	PacketCreate(&pack, SERVER_LOBBY_EXE_CHANCE);
	PacketWrite(&pack, packet_write8, v->exe_chance);
	RAssert(packet_sendtcp(v->server, v->id, &pack));

	PacketCreate(&pack, SERVER_PLAYER_JOINED);
	PacketWrite(&pack, packet_write16, v->id);
	PacketWrite(&pack, packet_writestr, v->nickname);
	PacketWrite(&pack, packet_write8, v->lobby_icon);
	PacketWrite(&pack, packet_write8, v->pet);
	server_broadcast_ex(v->server, &pack, v->id);

	if (v->server->state < ST_GAME)
		return lobby_state_join(v);
	else
		return game_state_join(v);

	return true;
}

bool server_state_handle(PeerData* v, Packet* packet)
{
	if (v->server->state < ST_GAME)
		return lobby_state_handle(v, packet);
	else
		return game_state_handletcp(v, packet);

	return true;
}

bool server_state_left(PeerData* v)
{
	Packet pack;
	PacketCreate(&pack, SERVER_PLAYER_LEFT);
	PacketWrite(&pack, packet_write16, v->id);
	server_broadcast(v->server, &pack);

	if (v->server->state < ST_GAME)
		return lobby_state_left(v);
	else
		return game_state_left(v);

	return true;
}

unsigned long server_cmd_parse(String* str)
{
	static const char* clr_list[] = CLRLIST;

	String current = { .len = 0 };
	uint8_t found_digit = 0;

	for (int i = 0; i < str->len; i++)
	{
		if (!found_digit && isspace(str->value[i]))
			continue;
		else
			found_digit = 1;
		
		if (isspace(str->value[i]))
			break;

		uint8_t invalid = 0;
		for (int j = 0; j < CLRLIST_LEN; j++)
		{
			if (str->value[i] == clr_list[j][0])
			{
				invalid = 1;
				break;
			}
		}

		if (invalid)
			continue;

		current.value[current.len++] = str->value[i];
	}
	current.value[current.len++] = '\0';

	unsigned int hash = 0;
	for (int i = 0; current.value[i] != '\0'; i++)
		hash = 31 * hash + current.value[i];

	return hash;
}


bool server_cmd_handle(Server* server, unsigned long hash, PeerData* v, String* msg)
{
	Packet pack;
	switch(hash)
	{
		default:
			return false;

		case CMD_BAN:
		{
			if (!v->op)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_RED "you aren't an operator."));
				break;
			}

			int ingame = server_total(v->server);
			if (ingame <= 1)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_RED "dude are you gonna ban yourself?"));
				break;
			}

			PacketCreate(&pack, SERVER_LOBBY_CHOOSEBAN);
			RAssert(packet_sendtcp(v->server, v->id, &pack));
			break;
		}

		case CMD_KICK:
		{
			if (!v->op)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_RED "you aren't an operator."));
				break;
			}

			int ingame = server_total(v->server);
			if (ingame <= 1)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_RED "dude are you gonna kick yourself?"));
				break;
			}

			PacketCreate(&pack, SERVER_LOBBY_CHOOSEKICK);
			RAssert(packet_sendtcp(v->server, v->id, &pack));
			break;
		}

		case CMD_OP:
		{
			if (!v->op)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_RED "you aren't an operator."));
				break;
			}

			int ingame = server_total(v->server);
			if (ingame <= 1)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_RED "you're already an operator tho??"));
				break;
			}

			PacketCreate(&pack, SERVER_LOBBY_CHOOSEOP);
			RAssert(packet_sendtcp(v->server, v->id, &pack));
			break;
		}

		/* Help message  */
		case CMD_HELP:
		{
			RAssert(server_send_msg(v->server, v->id, CLRCODE_GRA ".info" CLRCODE_RST " server info"));
			RAssert(server_send_msg(v->server, v->id, CLRCODE_GRA ".vk" CLRCODE_RST " vote kick"));
			RAssert(server_send_msg(v->server, v->id, CLRCODE_GRA ".vp" CLRCODE_RST " vote practice mode"));

			if (v->op)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_GRA ".map" CLRCODE_RST " choose map (0-18)"));
				RAssert(server_send_msg(v->server, v->id, CLRCODE_GRA ".kick" CLRCODE_RST " kick someone ig"));
				RAssert(server_send_msg(v->server, v->id, CLRCODE_GRA ".ban" CLRCODE_RST " ban someone ig"));
				RAssert(server_send_msg(v->server, v->id, CLRCODE_GRA ".op" CLRCODE_RST " op someone ig"));
			}

			break;
		}

		/* Information about the lobby */
		case CMD_INFO:
		{
			char msg[100];
			snprintf(msg, 100, "server " CLRCODE_RED "%d" CLRCODE_RST " of " CLRCODE_BLU "%d" CLRCODE_RST, v->server->id + 1, g_config.server_count);

			server_send_msg(v->server, v->id, "-----------------------");
			server_send_msg(v->server, v->id, CLRCODE_RED "better" CLRCODE_BLU "server" CLRCODE_RST " v" STRINGIFY(BUILD_VERSION));
			server_send_msg(v->server, v->id, "build from " CLRCODE_PUR  __DATE__ " " CLRCODE_GRN  __TIME__ CLRCODE_RST);
			server_send_msg(v->server, v->id, msg);
			server_send_msg(v->server, v->id, "-----------------------");
			server_send_msg(v->server, v->id, CLRCODE_GRA "type .help for command list" CLRCODE_RST);
			break;
		}
		
		/* Does he know? */
		case CMD_STINK:
		{
			char buff[36];
			if (sscanf(msg->value, ".stink %35s", buff) <= 0)
			{
				RAssert(server_send_msg(v->server, v->id, CLRCODE_RED "example:~ .stink baller"));
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
 	switch(type)
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
					PeerData* peer = (PeerData*)v->server->peers.ptr[i];
					if (!peer)
						continue;

					if (peer->id == pid)
					{
						RAssert(ban_add(peer->nickname.value, inet_ntoa(peer->addr.sin_addr), peer->udid.value));
						server_disconnect(v->server, peer->id, DR_BANNEDBYHOST, NULL);
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
					PeerData* peer = (PeerData*)v->server->peers.ptr[i];
					if (!peer)
						continue;

					if (peer->id == pid)
					{
						RAssert(timeout_set(peer->nickname.value, peer->udid.value, inet_ntoa(peer->addr.sin_addr), time(NULL) + 60));
						server_disconnect(v->server, peer->id, DR_KICKEDBYHOST, NULL);
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
					PeerData* peer = (PeerData*)v->server->peers.ptr[i];
					if (!peer)
						continue;

					if (peer->id == pid)
					{
						peer->op = 1;
						RAssert(op_add(peer->nickname.value, inet_ntoa(peer->addr.sin_addr)));
						server_send_msg(v->server, peer->id, CLRCODE_GRN "you're an operator now");
						break;
					}
				}
				break;
			}
	}

	return true;
}

bool server_send_msg(Server* server, uint16_t id, const char* message)
{
	Packet pack;
	PacketCreate(&pack, CLIENT_CHAT_MESSAGE);
	PacketWrite(&pack, packet_write16, 0);
	PacketWrite(&pack, packet_writestr, string_lower(__Str(message)));

	RAssert(packet_sendtcp(server, id, &pack));
	return true;
}

bool server_broadcast_msg(Server* server, const char* message)
{
	Packet pack;
	PacketCreate(&pack, CLIENT_CHAT_MESSAGE);
	PacketWrite(&pack, packet_write16, 0);
	PacketWrite(&pack, packet_writestr, string_lower(__Str(message)));

	server_broadcast(server, &pack);
	return true;
}