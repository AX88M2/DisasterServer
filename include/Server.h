#ifndef SERVER_H
#define SERVER_H

#include "Lib.h"
#include "Log.h"
#include "Vote.h"
#include "DyList.h"
#include "io/TcpListener.h"
#include "io/UdpListener.h"
#include "io/Packet.h"
#include "io/Threads.h"
#include "io/Time.h"
#include <stdbool.h>
#include <stdint.h>

struct Server;
typedef struct
{
	struct Server*		server;

	struct sockaddr_in	addr;
	int					id;

	/* State info */
	double				timeout;
	double				vote_cooldown;
	uint8_t				op;
	uint8_t				ready;
	uint8_t				in_game;
	uint8_t				can_vote;
	uint8_t				voted;

	/* Character */
	uint8_t				exe_chance;
	enum
	{
		CH_NONE = -1,

		CH_TAILS,
		CH_KNUX,
		CH_EGGMAN,
		CH_AMY,
		CH_CREAM,
		CH_SALLY
	} surv_char;

	enum
	{
		EX_NONE = -1,

		EX_ORIGINAL,
		EX_CHAOS,
		EX_EXETIOR,
		EX_EXELLER
	} exe_char;

	/* General info */
	uint8_t				lobby_icon;
	int8_t				pet;
	String				nickname;
	String				udid;
} PeerData;

/**
 * Lobby state
 * Split into 3 substates (Lobby, Map Vote and Character Select)
 */
typedef struct
{
	/* Lobby */
	double		countdown; /* Used for countdown in lobby */
	uint8_t		countdown_sec; /* Countdown time in seconds */

	Vote		vote; /* Current vote (see Vote.h) */
	PeerData	kick_target; /* Copied over info about the target */
	double		prac_countdown; /* Used for small delay before entering practice mode */

	/* Map Vote */
	uint8_t		maps[3]; /* Map set for map vote */
	uint8_t		votes[3]; /* Used to store vote count for each map */

	/* Character Select */
	int8_t		map; /* Map chosen in map vote */
	int			exe; /* Player chosen as an exe */
	uint8_t		avail[CH_SALLY+1]; /* Available characters */
} Lobby;

typedef enum
{
	BS_NONE, /* Big ring hasn't been spawned yet */
	BS_ACTIVATED, /* Big ring spawned AND is usable */
	BS_DEACTIVATED /* Big ring spawned, but cannot be used yet */
} BigRingState;

typedef struct
{
	int				exe; /* Current EXE's ID */
	int8_t			map; /* Current map */
	
	double			start_timeout;
	double			time;
	uint16_t		time_sec;
	double			end; /* Used for ending screen delay */

	uint8_t			started; /* Flag set when all players are ready */
	BigRingState	bring_state; /* Current state of the Big Ring */
	uint8_t			bring_loc; /* Random value used to determine Big Ring's spawn location */
	uint8_t			sudden_death; /* Flag set when time is <= 2 minutes */

	uint16_t		entid; /* Used as generator for next entity's ID (starts with 1) */
	DyList			entities;

	/* Rings */
	uint8_t			rings[256]; /* Lookup array used to find free location for the ring */
	uint8_t			ring_coff; /* Used to determine ring spawn speed on the map */

	DyList			players;

	#define			PLAYER_COOLCOUNT 10
	double			cooldowns[PLAYER_COOLCOUNT];
	Timer			tails_last_proj;
} Game;

// List of cooldown indicies

#define TAILS_RECHARGE 0
#define CREAM_RING_SPAWN 1
#define EXETIOR_BRING_SPAWN 2
#define EXELLER_CLONE_SPAWN 3
#define EGGTRACK_RECHARGE 4

typedef struct Server
{
	TcpListener* tcp;
	UdpListener udp;
	DyList		peers;

	uint16_t	id; // ID of the server

	// States
	Lobby		lobby;
	Game		game;
	int8_t		last_map; // Last map selected by map vote (-1 default)

	enum
	{
		ST_LOBBY,
		ST_MAPVOTE,
		ST_CHARSELECT,
		ST_GAME
	}			state;
	Mutex		state_lock;

	double		delta; // Delta time used for time/speed calculations
} Server;

// Used for API
typedef struct
{
	String		nickname;
	uint16_t	id;
	uint8_t		is_exe;
	int8_t		character;
} PlayerInfo;

typedef enum
{
	DR_FAILEDTOCONNECT,
	DR_KICKEDBYHOST,
	DR_BANNEDBYHOST,
	DR_VERMISMATCH,
	DR_TCPTIMEOUT,
	DR_UDPTIMEOUT,
	DR_PACKETSNOTRECV,
	DR_GAMESTARTED,
	DR_AFKTIMEOUT,
	DR_LOBBYFULL,
	DR_RATELIMITED,
	DR_SHUTDOWN,

	DR_DONTREPORT = 254,
	DR_OTHER = 255
} DisconnectReason;

#define TICKSPERSEC 60
#define BUILD_VERSION 1012
#define STR_HELPER(x) #x
#define STRINGIFY(x)  STR_HELPER(x)

bool 			server_send_msg			(Server* server, uint16_t id, const char* message);
bool 			server_broadcast_msg	(Server* server, const char* message);
bool 			server_msg_handle 		(Server* server, PacketType type, PeerData* v, Packet* packet);
bool 			server_cmd_handle 		(Server* server, unsigned long hash, PeerData* v, String* msg);
unsigned long 	server_cmd_parse 		(String* str);

bool	server_state_joined (PeerData* v);
bool	server_state_handle (PeerData* v, Packet* packet);
bool	server_state_left 	(PeerData* v);

bool	server_peerworker 	(PeerData* v);
bool	server_udpworker 	(Server* server);
bool	server_tick 		(Server* server);
bool	server_broadcast 	(Server* server, Packet* packet);
bool	server_broadcast_ex (Server* server, Packet* packet, uint16_t ignore);

SERVER_API int		server_total 		(Server* server);
SERVER_API int		server_ingame 		(Server* server);
SERVER_API uint8_t	server_state 		(Server* server);
SERVER_API bool		server_ban 			(Server* server, uint16_t id);
SERVER_API bool		server_op 			(Server* server, uint16_t id);
SERVER_API bool		server_timeout 		(Server* server, uint16_t id, double timeout);
SERVER_API bool		server_playerinfo 	(Server* server, int index, PlayerInfo* info);
SERVER_API bool		server_disconnect 	(Server* server, int id, DisconnectReason reason, const char* text);

#endif