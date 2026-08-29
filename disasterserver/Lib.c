#include <enet/enet.h>
#include <io/Dir.h>
#include <io/Threads.h>
#include <Lib.h>
#include <Log.h>
#include <Server.h>
#include <States.h>
#include <DyList.h>
#include <Config.h>

ThreadVar		g_threadName;
DyList			servers;
bool 			running = 0;

bool allocate_server(uint16_t base_port, uint16_t n)
{
	const Server templ = {
		.state = ST_LOBBY,
		.last_map = -1,
		.id = n,
		.running = true,
		.delta = 1,
		.game = {
			.exe = -1
		},
		.lobby = {
			.vote = {
				.ongoing = 0,
			},
			.prac_countdown = 0
		}
	};

	Server* server = malloc(sizeof(Server));
	RAssert(server);
	memcpy(server, &templ, sizeof(Server));

	for (int i = 0; i < MAP_COUNT; i++)
		server->map_pickrates[i] = 255;

	// Init lobby
	MutexCreate(server->state_lock);
	RAssert(dylist_create(&server->peers, 7));
	
	ENetAddress addr;
	addr.host = ENET_HOST_ANY;
	addr.port = base_port + n;
	server->host = enet_host_create(&addr, 50, 2, 0, 0);

	Info("Listening on port %d.", base_port + n);
	RAssert(server->host != NULL);
	RAssert(lobby_init(server));
	RAssert(dylist_push(&servers, server));

	return true;
}

bool disaster_init(void)
{
	if (running)
		return true;

	RAssert(enet_initialize() == 0);

	// Init global variables
	ThreadVarCreate(g_threadName);
	ThreadVarSet(g_threadName, "Main Thr");

#ifdef SYS_ANDROID
	log_hook(log_android);
#endif

	Info("--------------------------------");
	Info(LOG_RED "Better" LOG_BLU "Server " LOG_RST "v" STRINGIFY(BUILD_VERSION));
	Info("Build from " LOG_PUR __DATE__ " " LOG_GRN __TIME__);
	Info("(c) 2024 Team Exe Empire");
	Info("--------------------------------");
	Info("");

	RAssert(config_init());
	RAssert(log_init());

	RAssert(dylist_create(&servers, g_config.server_count));
	for (int32_t i = 0; i < g_config.server_count; i++)
		RAssert(allocate_server((uint16_t)g_config.port, i));
	
	return true;
}

int disaster_run(void)
{
	if (running)
		return 1;

	running = true;
	Debug("Entering main loop...");

	for(int32_t i = 0; i < g_config.server_count; i++)
	{
		Server* server = servers.ptr[i];
		if(!server)
			continue;

		Thread th;
		ThreadSpawn(th, server_worker, server);
	}

	// dont ask too many questions
	while (running)
	{
		ThreadSleep(100);
	}
	
	return 0;
}

void disaster_shutdown(void)
{
	if(!running)
		return;

	running = false;
	exit(0);
}

Server* disaster_get(int i)
{
	if (i < 0 || (size_t)i >= servers.capacity)
		return NULL;

	return (Server*)servers.ptr[i];
}

int disaster_count(void)
{
	return (int)servers.capacity;
}

bool disaster_server_lock(Server* server)
{
	RAssert(server);
	MutexLock(server->state_lock);
	return true;
}

bool disaster_server_unlock(Server* server)
{
	RAssert(server);
	MutexUnlock(server->state_lock);
	return true;
}

uint8_t disaster_server_state(Server* server)
{
	RAssert(server);
	return (uint8_t)server->state;
}

bool disaster_server_ban(Server* server, uint16_t id)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* v = (PeerData*)server->peers.ptr[i];
		if (!v)
			continue;

		if (v->id == id)
		{
			server_disconnect(server, v->peer, DR_BANNEDBYHOST, NULL);
			return ban_add(v->nickname.value, v->udid.value, v->ip.value);
		}
	}

	return false;
}

bool disaster_server_op(Server* server, uint16_t id)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* v = (PeerData*)server->peers.ptr[i];
		if (!v)
			continue;

		if (v->id == id)
		{
			v->op = true;

			server_send_msg(server, v->peer, CLRCODE_GRN "you're an operator now");
			return op_add(v->nickname.value, v->ip.value);
		}
	}

	return false;
}

bool disaster_server_timeout(Server* server, uint16_t id, double timeout)
{
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* v = (PeerData*)server->peers.ptr[i];
		if (!v)
			continue;

		if (v->id == id)
		{
			server_disconnect(server, v->peer, DR_KICKEDBYHOST, NULL);
			return timeout_set(v->nickname.value, v->udid.value, v->ip.value, time(NULL) + (uint64_t)(round(timeout)));
		}
	}

	return false;
}

bool disaster_server_peer(Server* server, int index, PeerInfo* info)
{
	info->character = -1;
	if (index < 0 || index >= server->peers.capacity)
		return false;
	else
	{
		PeerData* v = (PeerData*)server->peers.ptr[index];
		if (!v)
			return false;
		else
		{
			info->peer = v;
			info->is_exe = (server->game.exe == v->id);
			info->ip_addr = v->ip;

			if (v->in_game && server->state >= ST_GAME)
			{
				if (info->is_exe)
					info->character = v->exe_char;
				else
					info->character = v->surv_char;
			}
		}
	}

	return true;
}

bool disaster_server_peer_disconnect(Server* server, uint16_t id, DisconnectReason reason, const char* text)
{
	return server_disconnect_id(server, (int)id, reason, text);
}

int disaster_server_peer_count(Server* server)
{
	return server_total(server);
}

int disaster_server_peer_ingame(Server* server)
{
	return server_ingame(server);
}

int8_t disaster_game_map(Server* server)
{
	return server->game.map;
}

double disaster_game_time(Server* server)
{
	return server->game.time;
}

uint16_t disaster_game_time_sec(Server* server)
{
	return server->game.time_sec;
}