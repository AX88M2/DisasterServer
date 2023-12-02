#include <Lib.h>
#include <io/Threads.h>
#include <Log.h>
#include <Server.h>
#include <States.h>
#include <DyList.h>
#include <Config.h>

TcpListener		tcp;
DyList			servers;
uint8_t 		running;

bool allocate_server(uint16_t base_port, uint16_t n)
{
	const Server templ = {
		.state = ST_LOBBY,
		.last_map = -1,
		.id = n,
		.tcp = &tcp,
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

	// Init lobby
	MutexCreate(server->state_lock);
	RAssert(dylist_create(&server->peers, 7));
	RAssert(udp_open(&server->udp, base_port + n));
	RAssert(lobby_init(server));

	Thread th;
	ThreadSpawn(th, server_tick, server);

	Thread th2;
	ThreadSpawn(th2, server_udpworker, server);
	ThreadPrioritse(th2);

	RAssert(dylist_push(&servers, server));
	return true;
}

Server* find_free(void)
{
	Server* server = NULL;
	for (size_t i = 0; i < servers.capacity; i++)
	{
		Server* srv = (Server*)servers.ptr[i];
		if (!srv)
			continue;

		if (srv->peers.noitems < 7)
		{
			server = srv;
			break;
		}
	}
	return server;
}

bool disaster_init(void)
{
	// We first initialize and then get rid of fd 0
	// id 0 is taken by server so we cant allow using it
	#ifdef WIN32
		// Init WinSock2
		WSADATA wsaData;
		RAssert(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);

		FILE* fp = fopen("NUL", "r");
		int _idontcare = _dup2(_fileno(fp), _fileno(stdin));
		fclose(fp);
	#elif defined(__unix) || defined(__unix__)
		int fd = open("/dev/null", O_RDONLY);
		dup2(fd, STDIN_FILENO);
		close(fd);
	#endif

	Info("--------------------------------");
	Info("DisasterServer v" STRINGIFY(BUILD_VERSION));
	Info("Build from " __DATE__ " " __TIME__);
	Info("(c) 2023 Team Exe Empire");
	Info("--------------------------------");
	Info("");

	RAssert(config_init());

	if (!log_init())
		Warn("Logging to file is disabled.");

	RAssert(dylist_create(&servers, g_config.server_count));

	for (int32_t i = 0; i < g_config.server_count; i++)
		RAssert(allocate_server((uint16_t)g_config.udp_port, i));

	RAssert(tcp_open(&tcp, (uint16_t)g_config.tcp_port));
	RAssert(tcp_listen(&tcp));
	return true;
}

int disaster_run(void)
{
	running = 1;
	Info("Entering main loop...");

	Mutex servers_lock;
	MutexCreate(servers_lock);

	while (running)
	{
		PeerData* data = (PeerData*)malloc(sizeof(PeerData));
		RAssert(data);
		memset(data, 0, sizeof(PeerData));

		bool res = tcp_next(&tcp, &data->id, &data->addr);
		Server* server = find_free();

		if (!res)
		{
			Debug("!res probably shutdown?");
			free(data);
			continue;
		}

		if (!server)
		{
			Warn("No free servers found.");
			server_disconnect(server, data->id, DR_LOBBYFULL, NULL);
			close(data->id);
			free(data);
			continue;
		}

		data->server = server;

		Thread th;
		ThreadSpawn(th, server_peerworker, data);
	}
	
	return 0;
}


void disaster_shutdown(void)
{
	if(!running)
		return;

	running = 0;

	#ifdef WIN32
		int how = SD_BOTH;
	#else
		int how = SHUT_RDWR;
	#endif

	shutdown(tcp.fd, how);
}

int disaster_servers(void)
{
	return servers.capacity;
}

Server* disaster_get(int i)
{
	if (i < 0 || (size_t)i >= servers.capacity)
		return NULL;

	return (Server*)servers.ptr[i];
}
