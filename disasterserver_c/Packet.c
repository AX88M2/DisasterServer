#include <enet/enet.h>
#include <ctype.h>
#include <Packet.h>
#include <CMath.h>
#include <Server.h>

#ifdef __GNUC__ // GCC, clang...
	#define BYTESWAP_16(x) __builtin_bswap16((x))
	#define BYTESWAP_32(x) __builtin_bswap32((x))
	#define BYTESWAP_64(x) __builtin_bswap64((x))
#else
	#define BYTESWAP_16(x) _byteswap_ushort((x))
	#define BYTESWAP_32(x) _byteswap_ulong((x))
	#define BYTESWAP_64(x) _byteswap_uint64((x))
#endif

String string_new(const char* value)
{
	size_t len = strlen(value) + 1;
	String str = { .value = {0},  .len = (uint16_t)len};
	memcpy(str.value, value, len);

	return str;
}

size_t string_length(String* str)
{
	size_t len = 0;

	for (size_t i = 0; i < str->len; i++)
	{
		if (str->value[i] == '\0')
			break;

		len += (str->value[i] & 0xc0) != 0x80;
	}

	return len;
}

String string_lower(String str)
{
	for (int i = 0; i < str.len; i++)
		str.value[i] = (char)tolower(str.value[i]);

	return str;
}

inline const char * getPacketTypeName(PacketType type) {
	switch (type) {
		case IDENTITY: return "IDENTITY";
		case SERVER_IDENTITY_RESPONSE: return "SERVER_IDENTITY_RESPONSE";
		case SERVER_PLAYER_JOINED: return "SERVER_PLAYER_JOINED";
		case SERVER_PLAYER_LEFT: return "SERVER_PLAYER_LEFT";
		case SERVER_PLAYER_FORCE_DISCONNECT: return "SERVER_PLAYER_FORCE_DISCONNECT";
		case SERVER_WAITING_PLAYER_INFO: return "SERVER_WAITING_PLAYER_INFO";
		case SERVER_LOBBY_READY_STATE: return "SERVER_LOBBY_READY_STATE";
		case SERVER_LOBBY_EXE: return "SERVER_LOBBY_EXE";
		case SERVER_LOBBY_COUNTDOWN: return "SERVER_LOBBY_COUNTDOWN";
		case SERVER_LOBBY_EXE_CHANGE: return "SERVER_LOBBY_EXE_CHANGE";
		case SERVER_LOBBY_CHARACTER_CHANGE: return "SERVER_LOBBY_CHARACTER_CHANGE";
		case SERVER_LOBBY_CHARACTER_RESPONSE: return "SERVER_LOBBY_CHARACTER_RESPONSE";
		case SERVER_LOBBY_EXECHARACTER_RESPONSE: return "SERVER_LOBBY_EXECHARACTER_RESPONSE";
		case SERVER_LOBBY_GAME_START: return "SERVER_LOBBY_GAME_START";
		case SERVER_LOBBY_PLAYER: return "SERVER_LOBBY_PLAYER";
		case SERVER_LOBBY_EXE_CHANCE: return "SERVER_LOBBY_EXE_CHANCE";
		case SERVER_LOBBY_CORRECT: return "SERVER_LOBBY_CORRECT";
		case SERVER_LOBBY_CHOOSEVOTEKICK: return "SERVER_LOBBY_CHOOSEVOTEKICK";
		case SERVER_LOBBY_CHOOSEBAN: return "SERVER_LOBBY_CHOOSEBAN";
		case SERVER_LOBBY_CHOOSEKICK: return "SERVER_LOBBY_CHOOSEKICK";
		case SERVER_LOBBY_CHOOSEOP: return "SERVER_LOBBY_CHOOSEOP";
		case SERVER_LOBBY_CHANGELOBBY: return "SERVER_LOBBY_CHANGELOBBY";
		case SERVER_CHAR_TIME_SYNC: return "SERVER_CHAR_TIME_SYNC";
		case SERVER_VOTE_MAPS: return "SERVER_VOTE_MAPS";
		case SERVER_VOTE_SET: return "SERVER_VOTE_SET";
		case SERVER_VOTE_TIME_SYNC: return "SERVER_VOTE_TIME_SYNC";
		case SERVER_GAME_PLAYERS_READY: return "SERVER_GAME_PLAYERS_READY";
		case SERVER_GAME_EXE_WINS: return "SERVER_GAME_EXE_WINS";
		case SERVER_GAME_SURVIVOR_WIN: return "SERVER_GAME_SURVIVOR_WIN";
		case SERVER_GAME_SPAWN_RING: return "SERVER_GAME_SPAWN_RING";
		case SERVER_GAME_PLAYER_ESCAPED: return "SERVER_GAME_PLAYER_ESCAPED";
		case SERVER_GAME_BACK_TO_LOBBY: return "SERVER_GAME_BACK_TO_LOBBY";
		case SERVER_GAME_TIME_SYNC: return "SERVER_GAME_TIME_SYNC";
		case SERVER_GAME_TIME_OVER: return "SERVER_GAME_TIME_OVER";
		case SERVER_GAME_PING: return "SERVER_GAME_PING";
		case SERVER_PLAYER_DEATH_STATE: return "SERVER_PLAYER_DEATH_STATE";
		case SERVER_GAME_DEATHTIMER_TICK: return "SERVER_GAME_DEATHTIMER_TICK";
		case SERVER_GAME_DEATHTIMER_END: return "SERVER_GAME_DEATHTIMER_END";
		case SERVER_REQUEST_INFO: return "SERVER_REQUEST_INFO";
		case SERVER_HEARTBEAT: return "SERVER_HEARTBEAT";
		case SERVER_PONG: return "SERVER_PONG";
		case SERVER_FORCE_DAMAGE: return "SERVER_FORCE_DAMAGE";
		case SERVER_GAME_RING_READY: return "SERVER_GAME_RING_READY";
		case SERVER_PLAYER_BACKTRACK: return "SERVER_PLAYER_BACKTRACK";
		case SERVER_TPROJECTILE_STATE: return "SERVER_TPROJECTILE_STATE";
		case SERVER_ETRACKER_STATE: return "SERVER_ETRACKER_STATE";
		case SERVER_ERECTOR_BRING_SPAWN: return "SERVER_ERECTOR_BRING_SPAWN";
		case SERVER_RMZSLIME_STATE: return "SERVER_RMZSLIME_STATE";
		case SERVER_RMZSLIME_RINGBONUS: return "SERVER_RMZSLIME_RINGBONUS";
		case SERVER_RMZSHARD_STATE: return "SERVER_RMZSHARD_STATE";
		case SERVER_LCEYE_STATE: return "SERVER_LCEYE_STATE";
		case SERVER_LCCHAIN_STATE: return "SERVER_LCCHAIN_STATE";
		case SERVER_NPCONTROLLER_STATE: return "SERVER_NPCONTROLLER_STATE";
		case SERVER_KAFMONITOR_STATE: return "SERVER_KAFMONITOR_STATE";
		case SERVER_YCRSMOKE_STATE: return "SERVER_YCRSMOKE_STATE";
		case SERVER_YCRSMOKE_READY: return "SERVER_YCRSMOKE_READY";
		case SERVER_MOVINGSPIKE_STATE: return "SERVER_MOVINGSPIKE_STATE";
		case SERVER_RING_STATE: return "SERVER_RING_STATE";
		case SERVER_RING_COLLECTED: return "SERVER_RING_COLLECTED";
		case SERVER_ACT9WALL_STATE: return "SERVER_ACT9WALL_STATE";
		case SERVER_NAPBALL_STATE: return "SERVER_NAPBALL_STATE";
		case SERVER_NAPICE_STATE: return "SERVER_NAPICE_STATE";
		case SERVER_PFLIFT_STATE: return "SERVER_PFLIFT_STATE";
		case SERVER_BRING_STATE: return "SERVER_BRING_STATE";
		case SERVER_BRING_COLLECTED: return "SERVER_BRING_COLLECTED";
		case SERVER_VVLCOLUMN_STATE: return "SERVER_VVLCOLUMN_STATE";
		case SERVER_VVVASE_STATE: return "SERVER_VVVASE_STATE";
		case SERVER_GHZTHUNDER_STATE: return "SERVER_GHZTHUNDER_STATE";
		case SERVER_TCGOM_STATE: return "SERVER_TCGOM_STATE";
		case SERVER_EXELLERCLONE_STATE: return "SERVER_EXELLERCLONE_STATE";
		case SERVER_DTTAILSDOLL_STATE: return "SERVER_DTTAILSDOLL_STATE";
		case SERVER_DTBALL_STATE: return "SERVER_DTBALL_STATE";
		case SERVER_DTASS_STATE: return "SERVER_DTASS_STATE";
		case SERVER_HDDOOR_STATE: return "SERVER_HDDOOR_STATE";
		case SERVER_WDLATERN_ACTIVATE: return "SERVER_WDLATERN_ACTIVATE";
		case SERVER_FART_STATE: return "SERVER_FART_STATE";
		case SERVER_MJLAVA_STATE: return "SERVER_MJLAVA_STATE";
		case SERVER_MJJUDGER_STATE: return "SERVER_MJJUDGER_STATE";
		case SERVER_MJCRYSTAL_STATE: return "SERVER_MJCRYSTAL_STATE";
		case CLIENT_ETRACKER: return "CLIENT_ETRACKER";
		case CLIENT_ETRACKER_ACTIVATED: return "CLIENT_ETRACKER_ACTIVATED";
		case CLIENT_TPROJECTILE: return "CLIENT_TPROJECTILE";
		case CLIENT_TPROJECTILE_HIT: return "CLIENT_TPROJECTILE_HIT";
		case CLIENT_TPROJECTILE_STARTCHARGE: return "CLIENT_TPROJECTILE_STARTCHARGE";
		case CLIENT_ERECTOR_BALLS: return "CLIENT_ERECTOR_BALLS";
		case CLIENT_ERECTOR_BRING_SPAWN: return "CLIENT_ERECTOR_BRING_SPAWN";
		case CLIENT_EXELLER_SPAWN_CLONE: return "CLIENT_EXELLER_SPAWN_CLONE";
		case CLIENT_EXELLER_TELEPORT_CLONE: return "CLIENT_EXELLER_TELEPORT_CLONE";
		case CLIENT_MERCOIN_BONUS: return "CLIENT_MERCOIN_BONUS";
		case CLIENT_RMZSLIME_HIT: return "CLIENT_RMZSLIME_HIT";
		case CLIENT_LCEYE_REQUEST_ACTIVATE: return "CLIENT_LCEYE_REQUEST_ACTIVATE";
		case CLIENT_KAFMONITOR_ACTIVATE: return "CLIENT_KAFMONITOR_ACTIVATE";
		case CLIENT_RING_COLLECTED: return "CLIENT_RING_COLLECTED";
		case CLIENT_RING_BROKE: return "CLIENT_RING_BROKE";
		case CLIENT_BRING_COLLECTED: return "CLIENT_BRING_COLLECTED";
		case CLIENT_NAPICE_ACTIVATE: return "CLIENT_NAPICE_ACTIVATE";
		case CLIENT_SPRING_USE: return "CLIENT_SPRING_USE";
		case CLIENT_PFLIT_ACTIVATE: return "CLIENT_PFLIT_ACTIVATE";
		case CLIENT_VVVASE_BREAK: return "CLIENT_VVVASE_BREAK";
		case CLIENT_RMZSHARD_COLLECT: return "CLIENT_RMZSHARD_COLLECT";
		case CLIENT_RMZSHARD_LAND: return "CLIENT_RMZSHARD_LAND";
		case CLIENT_DTASS_ACTIVATE: return "CLIENT_DTASS_ACTIVATE";
		case CLIENT_HDDOOR_TOGGLE: return "CLIENT_HDDOOR_TOGGLE";
		case CLIENT_FART_PUSH: return "CLIENT_FART_PUSH";
		case CLIENT_LOBBY_READY_STATE: return "CLIENT_LOBBY_READY_STATE";
		case CLIENT_REQUESTED_INFO: return "CLIENT_REQUESTED_INFO";
		case CLIENT_PLAYER_DATA: return "CLIENT_PLAYER_DATA";
		case CLIENT_PLAYER_HURT: return "CLIENT_PLAYER_HURT";
		case CLIENT_SOUND_EMIT: return "CLIENT_SOUND_EMIT";
		case CLIENT_PING: return "CLIENT_PING";
		case CLIENT_REVIVAL_PROGRESS: return "CLIENT_REVIVAL_PROGRESS";
		case CLIENT_PLAYER_HEAL: return "CLIENT_PLAYER_HEAL";
		case CLIENT_PLAYER_HEAL_PART: return "CLIENT_PLAYER_HEAL_PART";
		case SERVER_REVIVAL_PROGRESS: return "SERVER_REVIVAL_PROGRESS";
		case SERVER_REVIVAL_STATUS: return "SERVER_REVIVAL_STATUS";
		case SERVER_REVIVAL_RINGSUB: return "SERVER_REVIVAL_RINGSUB";
		case SERVER_REVIVAL_REVIVED: return "SERVER_REVIVAL_REVIVED";
		case CLIENT_REQUEST_CHARACTER: return "CLIENT_REQUEST_CHARACTER";
		case CLIENT_REQUEST_EXECHARACTER: return "CLIENT_REQUEST_EXECHARACTER";
		case CLIENT_VOTE_REQUEST: return "CLIENT_VOTE_REQUEST";
		case CLIENT_PLAYER_DEATH_STATE: return "CLIENT_PLAYER_DEATH_STATE";
		case CLIENT_PLAYER_ESCAPED: return "CLIENT_PLAYER_ESCAPED";
		case SERVER_PLAYER_ESCAPED: return "SERVER_PLAYER_ESCAPED";
		case CLIENT_LOBBY_PLAYERS_REQUEST: return "CLIENT_LOBBY_PLAYERS_REQUEST";
		case CLIENT_CREAM_SPAWN_RINGS: return "CLIENT_CREAM_SPAWN_RINGS";
		case CLIENT_SPAWN_EFFECT: return "CLIENT_SPAWN_EFFECT";
		case CLIENT_CHAT_MESSAGE: return "CLIENT_CHAT_MESSAGE";
		case CLIENT_LOBBY_CHOOSEVOTEKICK: return "CLIENT_LOBBY_CHOOSEVOTEKICK";
		case CLIENT_LOBBY_CHOOSEBAN: return "CLIENT_LOBBY_CHOOSEBAN";
		case CLIENT_LOBBY_CHOOSEKICK: return "CLIENT_LOBBY_CHOOSEKICK";
		case CLIENT_LOBBY_CHOOSEOP: return "CLIENT_LOBBY_CHOOSEOP";
		case CLIENT_PLAYER_PALETTE: return "CLIENT_PLAYER_PALETTE";
		case CLIENT_PET_PALETTE: return "CLIENT_PET_PALETTE";
		case SERVER_RESULTS: return "SERVER_RESULTS";
		case SERVER_RESULTS_DATA: return "SERVER_RESULTS_DATA";
		case CLIENT_RESULTS_REQUEST: return "CLIENT_RESULTS_REQUEST";
		case CLIENT_STATS_REPORT: return "CLIENT_STATS_REPORT";
		case SERVER_PREIDENTITY: return "SERVER_PREIDENTITY";
		case SERVER_FELLA: return "SERVER_FELLA";
		case CLIENT_PLAYER_POTATER: return "CLIENT_PLAYER_POTATER";
		default: return "<Unknown>";
	}
}

bool packet_new(Packet* packet, PacketType type)
{
	RAssert(packet);
	packet->len = 0;
	packet->pos = 0;

	PacketWrite(packet, packet_write8, 0);
	PacketWrite(packet, packet_write8, (uint8_t)type);

	Debug("Created packet %s", getPacketTypeName(type));

	return true;
}

Packet packet_from(ENetPacket* packet)
{
	Packet pack = (Packet) { .pos = 0,  .len = (uint8_t)packet->dataLength };
	memcpy(pack.buff, packet->data, (uint8_t)packet->dataLength);
	enet_packet_destroy(packet);
	return pack;
}

bool packet_send(ENetPeer* peer, Packet* packet, bool reliable)
{
	PeerData* data = (PeerData*)peer->data;
	if(data && data->disconnecting)
		return true;

	if (packet->buff[1] != SERVER_HEARTBEAT) {
		Debug("Sending packet %s", getPacketTypeName(packet->buff[1]));
	}

	packet->pos = 0;
	ENetPacket* pack = enet_packet_create(packet, packet->len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
	return enet_peer_send(peer, reliable ? 0 : 1, pack) == 0;
}

bool packet_send_id(struct Server* server, uint16_t id, Packet* packet, bool reliable)
{
	packet->pos = 0;
	ENetPacket* pack = enet_packet_create(packet, packet->len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

	for(int32_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* data = (PeerData*)server->peers.ptr[i];
		if(!data)
			continue;

		if(data->id != id)
			continue;
			
		return enet_peer_send(data->peer, reliable ? 0 : 1, pack) == 0;
	}

	return false;
}

bool packet_seek(Packet* packet, int wh)
{
	RAssert(wh >= 0);
	RAssert(wh < PACKET_MAXSIZE);
	RAssert(wh < packet->len);

	packet->pos = wh;
	return true;
}

bool packet_read8(Packet* packet, uint8_t* out)
{
	RAssert(packet->pos < packet->len);
	*out = packet->buff[packet->pos++];

	return true;
}

bool packet_read16(Packet* packet, uint16_t* out)
{
	RAssert(packet->pos < packet->len);
	*out = *((int16_t*)&packet->buff[packet->pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_16(*out);
#endif

	packet->pos += 2;

	return true;
}

bool packet_read32(Packet* packet, uint32_t* out)
{
	RAssert(packet->pos < packet->len);
	*out = *((int32_t*)&packet->buff[packet->pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_32(*out);
#endif

	packet->pos += 4;

	return true;
}

bool packet_read64(Packet* packet, uint64_t* out)
{
	RAssert(packet->pos < packet->len);
	*out = *((int64_t*)&packet->buff[packet->pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_64(*out);
#endif

	packet->pos += 8;

	return true;
}

bool packet_readfloat(Packet* packet, float* out)
{
	RAssert(packet->pos < packet->len);
	*out = *((float*)&packet->buff[packet->pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_32(*out);
#endif

	packet->pos += 4;

	return true;
}

bool packet_readdouble(Packet* packet, double* out)
{
	RAssert(packet->pos < packet->len);
	*out = *((float*)&packet->buff[packet->pos]);

#ifdef SYS_BIG_ENDIAN
	* out = BYTESWAP_32(*out);
#endif

	packet->pos += 8;

	return true;
}

bool packet_readstr(Packet* packet, String* out)
{
	out->len = 0;

	while (1)
	{
		RAssert(packet->pos < packet->len);
		RAssert(out->len < 128);

		char ch = packet->buff[packet->pos];
		out->value[out->len] = ch;

		packet->pos++;
		out->len++;

		if (ch == '\0')
			break;
	}

	return true;
}

bool packet_write8(Packet* packet, uint8_t value)
{
	RAssert(packet->pos + 1 < PACKET_MAXSIZE);

	if (packet->pos + 1 >= packet->len)
		packet->len++;

	packet->buff[packet->pos++] = value;
	return true;
}

bool packet_write16(Packet* packet, uint16_t value)
{
	RAssert(packet->pos + 2 < PACKET_MAXSIZE);

	if (packet->pos + 2 >= packet->len)
		packet->len += 2;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_16(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	packet->buff[packet->pos++] = ptr[0];
	packet->buff[packet->pos++] = ptr[1];
	return true;
}

bool packet_write32(Packet* packet, uint32_t value)
{
	RAssert(packet->pos + 4 < PACKET_MAXSIZE);

	if (packet->pos + 4 >= packet->len)
		packet->len += 4;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_32(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	packet->buff[packet->pos++] = ptr[0];
	packet->buff[packet->pos++] = ptr[1];
	packet->buff[packet->pos++] = ptr[2];
	packet->buff[packet->pos++] = ptr[3];
	return true;
}

bool packet_write64(Packet* packet, uint64_t value)
{
	RAssert(packet->pos + 8 < PACKET_MAXSIZE);

	if (packet->pos + 8 >= packet->len)
		packet->len += 8;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_64(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	packet->buff[packet->pos++] = ptr[0];
	packet->buff[packet->pos++] = ptr[1];
	packet->buff[packet->pos++] = ptr[2];
	packet->buff[packet->pos++] = ptr[3];
	packet->buff[packet->pos++] = ptr[4];
	packet->buff[packet->pos++] = ptr[5];
	packet->buff[packet->pos++] = ptr[6];
	packet->buff[packet->pos++] = ptr[7];
	return true;
}

bool packet_writefloat(Packet* packet, float value)
{
	RAssert(packet->pos + 4 < PACKET_MAXSIZE);

	if (packet->pos + 4 >= packet->len)
		packet->len += 4;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_32(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	packet->buff[packet->pos++] = ptr[0];
	packet->buff[packet->pos++] = ptr[1];
	packet->buff[packet->pos++] = ptr[2];
	packet->buff[packet->pos++] = ptr[3];
	return true;
}

bool packet_writedouble(Packet* packet, double value)
{
	RAssert(packet->pos + 8 < PACKET_MAXSIZE);

	if (packet->pos + 8 >= packet->len)
		packet->len += 8;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_64(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	packet->buff[packet->pos++] = ptr[0];
	packet->buff[packet->pos++] = ptr[1];
	packet->buff[packet->pos++] = ptr[2];
	packet->buff[packet->pos++] = ptr[3];
	packet->buff[packet->pos++] = ptr[4];
	packet->buff[packet->pos++] = ptr[5];
	packet->buff[packet->pos++] = ptr[6];
	packet->buff[packet->pos++] = ptr[7];
	return true;
}

bool packet_writestr(Packet* packet, String value)
{
	for (uint8_t i = 0; i < value.len; i++)
		RAssert(packet_write8(packet, value.value[i]));

	return true;
}
