#ifndef LIB_H
#define LIB_H
#include <Api.h>
#include <enet/enet.h>
#include <io/Threads.h>
#include <DyList.h>
#include <Packet.h>

typedef enum
{
	DR_FAILEDTOCONNECT,
	DR_KICKEDBYHOST,
	DR_BANNEDBYHOST,
	DR_VERMISMATCH,
	DR_SERVERTIMEOUT,
	DR_PACKETSNOTRECV,
	DR_GAMESTARTED,
	DR_AFKTIMEOUT,
	DR_LOBBYFULL,
	DR_RATELIMITED,
	DR_SHUTDOWN,
	DR_IPINUSE,

	DR_DONTREPORT = 254,
	DR_OTHER = 255
} DisconnectReason;

struct Server;
struct PeerData;
typedef struct
{
	struct PeerData*	peer;
	String				ip_addr;
	bool				is_exe;
	int8_t				character;
} PeerInfo;

SERVER_API bool				disaster_init 	 (void);
SERVER_API int				disaster_run 	 (void);
SERVER_API void 			disaster_shutdown(void);

// API functions
SERVER_API struct Server*	disaster_get					(int);
SERVER_API int				disaster_count					(void);
SERVER_API bool				disaster_server_lock			(struct Server* server);
SERVER_API bool				disaster_server_unlock			(struct Server* server);
SERVER_API uint8_t			disaster_server_state			(struct Server* server);
SERVER_API bool				disaster_server_ban				(struct Server* server, uint16_t);
SERVER_API bool				disaster_server_op				(struct Server* server, uint16_t);
SERVER_API bool				disaster_server_timeout			(struct Server* server, uint16_t, double);
SERVER_API bool				disaster_server_peer			(struct Server*, int, PeerInfo*);
SERVER_API bool				disaster_server_peer_disconnect	(struct Server*, uint16_t, DisconnectReason, const char*);
SERVER_API int				disaster_server_peer_count		(struct Server*);
SERVER_API int				disaster_server_peer_ingame		(struct Server*);
SERVER_API int8_t			disaster_game_map				(struct Server*);
SERVER_API double			disaster_game_time				(struct Server*);
SERVER_API uint16_t			disaster_game_time_sec			(struct Server*);

#endif