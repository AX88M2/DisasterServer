#ifndef SERVER_H
#define SERVER_H

#include <Auth.h>
#include <DyList.h>
#include <Lib.h>
#include <Log.h>
#include <Vote.h>
#include <DyList.h>
#include <Player.h>
#include <Packet.h>
#include <io/Threads.h>
#include <io/Time.h>
#include <enet/enet.h>
#include <stdbool.h>
#include <stdint.h>

#define TICKSPERSEC 60
#define BUILD_VERSION 1101

#define STR_HELPER(x) #x
#define STRINGIFY(x) STR_HELPER(x)

struct Server;
struct auth_peer_data;
typedef struct PeerData
{
	uint16_t id;
	String ip;
	ENetPeer *peer;

	/* General info */
	Player plr;
	String nickname;
	String udid;
	uint8_t lobby_icon;
	int8_t pet;

	bool verified;
	bool in_game;
	bool op;
	bool ready;
	bool mod_tool;
	bool is_mobile;
	bool can_vote;
	bool voted;
	bool disconnecting;

	auth_peer_data auth;

	/* Character */
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

	bool should_timeout;

	/* State info */
	uint8_t exe_chance;
	double timeout;
	double vote_cooldown;

	struct Server *server;
} PeerData;

typedef struct
{
	/* Lobby */
	double countdown;
	double prac_countdown;
	uint8_t countdown_sec;
	Vote vote;
	PeerData kick_target;

	/* Map Vote */
	uint8_t maps[3];
	uint8_t votes[3];

	/* Character Select */
	int8_t map;
	uint16_t exe;
	bool avail[CH_SALLY + 1];
} Lobby;

typedef enum
{
	BS_NONE,
	BS_DEACTIVATED,
	BS_ACTIVATED,
} BigRingState;

typedef enum
{
	ED_EXEWIN,
	ED_SURVWIN,
	ED_TIMEOVER
} Ending;

typedef struct
{
	int exe;
	int8_t map;
	bool started;
	bool sudden_death;

	double start_timeout;
	double time;
	double elapsed;
	uint16_t time_sec;
	double end;
	Ending ending;

	BigRingState bring_state;
	uint8_t bring_loc;

	/* Entities */
	uint16_t entid;
	DyList entities;

	/* Rings */
	bool rings[256];
	uint8_t ring_coff;

	/* Lists */
	DyList left;

#define PLAYER_COOLCOUNT 6
	double cooldowns[PLAYER_COOLCOUNT];
	TimeStamp tails_last_proj;
} Game;

// List of cooldown indicies
#define TAILS_RECHARGE 0
#define CREAM_RING_SPAWN 1
#define EXETIOR_BRING_SPAWN 2
#define EXELLER_CLONE_SPAWN 3
#define EGGTRACK_RECHARGE 4
#define ETAILS_RECHARGE 5

typedef struct Results
{
	double countdown;
} Results;

typedef struct Server
{
	uint16_t id;
	bool running;

	enum
	{
		ST_LOBBY,
		ST_MAPVOTE,
		ST_CHARSELECT,
		ST_GAME,
		ST_RESULTS
	} state;
	Mutex state_lock;

	/* States */
	Lobby lobby;
	Game game;
	Results results;

	int8_t last_map;
	int16_t map_pickrates[30];

	double delta;
	DyList peers;
	ENetHost *host;
} Server;

bool server_state_joined(PeerData *v);
bool server_state_left(PeerData *v);
bool server_state_handle(PeerData *v, Packet *packet);
bool server_msg_handle(Server *server, PacketType type, PeerData *v, Packet *packet);
bool server_cmd_handle(Server *server, unsigned long hash, PeerData *v, String *msg);
unsigned long server_cmd_parse(String *string);

bool server_worker(Server *server);
bool server_broadcast(Server *server, Packet *packet, bool reliable);
bool server_broadcast_ex(Server *server, Packet *packet, bool reliable, uint16_t ignore);
bool server_send_msg(Server *server, ENetPeer *peer, const char *message);
bool server_broadcast_msg(Server *server, const char *message);

int server_total(Server *server);
int server_ingame(Server *server);
PeerData *server_find_peer(Server *server, uint16_t id);
bool server_disconnect(Server *server, ENetPeer *peer, DisconnectReason reason, const char *text);
bool server_disconnect_id(Server *server, uint16_t id, DisconnectReason reason, const char *text);

#endif
