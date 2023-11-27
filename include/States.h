#ifndef LOBBY_H
#define LOBBY_H
#include "Server.h"
#include "Log.h"
#include "Maps.h"
#include "Player.h"

#define AssertOrDisconnect(server, x) if(!(x)) { server_disconnect(server, v->id, DR_OTHER, "AssertOrDisconnect(" #x ") failed!"); RAssert(x); }
#define ENTITY_BODY \
char tag[16];\
uint16_t id;\
Vector2 pos;\
bool (*init)(Server*, struct Entity*);\
bool (*tick)(Server*, struct Entity*);\
bool (*uninit)(Server*, struct Entity*);

struct Entity;
typedef struct Entity
{
	ENTITY_BODY
} Entity;
#define MakeEntity(tag, x, y) tag, 0, (Vector2){ x, y },

#define CMD_HELP 45680751
#define CMD_MAP 1478254
#define CMD_STINK 1426706039
#define CMD_VK 47971
#define CMD_VP 47976
#define CMD_BAN 1467681
#define CMD_KICK 45773684
#define CMD_OP 47759
#define CMD_YES 1489913
#define CMD_Y 1547
#define CMD_NO 47727
#define CMD_N 1536
#define CMD_INFO 45719004

bool lobby_init				(Server* server);
bool lobby_state_join		(PeerData* v);
bool lobby_state_left		(PeerData* v);
bool lobby_state_handle		(PeerData* v, Packet* packet);
bool lobby_state_tick		(Server* server);


bool mapvote_init			(Server* server);
bool mapvote_state_join		(PeerData * v);
bool mapvote_state_left		(PeerData * v);
bool mapvote_state_handle	(PeerData* v, Packet* packet);
bool mapvote_state_tick		(Server* server);

bool charselect_init			(int8_t map, Server* server);
bool charselect_state_join		(PeerData* v);
bool charselect_state_left		(PeerData* v);
bool charselect_state_handle	(PeerData* v, Packet * packet);
bool charselect_state_tick		(Server* server);

bool	game_init				(int exe, int8_t map, Server* server);
bool	game_uninit				(Server* server);
bool	game_spawn				(Server* server, Entity* entity, size_t size, Entity** out);
bool	game_despawn			(Server* server, Entity** out, uint16_t id);
int		game_find				(Server* server, Entity** out, char* tag, size_t count);

Player* game_findplr			(Server* server, uint16_t id);

bool	game_bigring			(Server* server, BigRingState state);
bool	game_state_join			(PeerData* v);
bool	game_state_left			(PeerData* v);
bool	game_state_handletcp	(PeerData* v, Packet* packet);
bool	game_state_handleudp	(Server* server, IpAddr* addr, Packet* packet);
bool	game_state_tick			(Server* server);

/* UDP broadcast */
void	game_broadcast			(Server* server, Packet* packet);
void	game_broadcast_ex		(Server* server, Packet* packet, IpAddr* addr);

/* Should be used outside the server */
SERVER_API int8_t	game_map(Server* server);
SERVER_API double	game_time(Server* server);
SERVER_API uint16_t game_timesec(Server* server);

#endif