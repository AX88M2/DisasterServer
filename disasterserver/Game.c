#include "Player.h"
#include <States.h>
#include <CMath.h>
#include <time.h>
#include <DyList.h>
#include <Server.h>
#include <Colors.h>
#include <entities/Ring.h>
#include <entities/CreamRing.h>
#include <entities/BlackRing.h>
#include <entities/TailsProjectile.h>
#include <entities/EggmanTracker.h>
#include <entities/ExellerClone.h>

typedef enum
{
	ED_EXEWIN,
	ED_SURVWIN,
	ED_TIMEOVER
} Ending;

bool game_end(Server* server, Ending ending)
{
	if (server->game.end > 0)
		return true;

	Packet packet;
	switch (ending)
	{
		case ED_EXEWIN:
		{
			PacketCreate(&packet, SERVER_GAME_EXE_WINS);
			Info("Ending is ED_EXEWIN");
			break;
		}

		case ED_SURVWIN:
		{
			PacketCreate(&packet, SERVER_GAME_SURVIVOR_WIN);
			Info("Ending is ED_SURVWIN");
			break;
		}

		case ED_TIMEOVER:
		{
			PacketCreate(&packet, SERVER_GAME_TIME_OVER);
			Info("Ending is ED_TIMEOVER");
			break;
		}
	}

	server_broadcast(server, &packet);
	server->game.end = 5 * TICKSPERSEC;
	return true;
}

bool game_state_check(Server* server)
{
	if (server->game.end > 0)
		return true;

	uint8_t escaped = 0;
	uint8_t dead = 0;
	uint8_t exes = 0;

	int total = 0;
	{
		for (size_t i = 0; i < server->game.players.capacity; i++)
		{
			Player* player = (Player*)server->game.players.ptr[i];
			if (!player)
				continue;

			if (player->flags & PLAYER_ESCAPED)
				escaped++;

			if (player->flags & PLAYER_DEAD || player->flags & PLAYER_DEMONIZED)
				dead++;

			if (player->id == server->game.exe)
				exes++;
		}

		total = server_ingame(server);
		total -= (exes + dead + escaped);
	}

	if (total <= 0)
	{
		if (escaped > 0)
		{
			RAssert(game_end(server, ED_SURVWIN));
		}
		else
		{
			RAssert(game_end(server, ED_EXEWIN));
		}
	}

	return true;
}

bool game_init(int exe, int8_t map, Server* server)
{
	Debug("Attepting to enter ST_GAME...");
	bool res = true;

	server->state = ST_GAME;
	server->game = (Game)
	{
		.map = map,
		.exe = exe,
		.bring_state = BS_NONE,
		.bring_loc = rand() % 255,
		.sudden_death = 0,
		.started = 0,
		.end = 0.0,
		.entid = 0,
		.time = TICKSPERSEC,
		.start_timeout = 15 * TICKSPERSEC
	};
	time_start(&server->game.tails_last_proj);

	// Setup rings
	memset(server->game.rings, 0, 256 * sizeof(uint8_t));

	if (!dylist_create(&server->game.entities, 3000))
		res = false;
	Debug("Entity list created.");

	if (!dylist_create(&server->game.players, 7))
		res = false;
	Debug("Player list created.");

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (!peer->in_game)
			continue;

		Player* plr = (Player*)malloc(sizeof(Player));
		RAssert(plr);

		memset(plr, 0, sizeof(Player));
		plr->id = peer->id;
		plr->ready = 0;

		for (int i = 0; i < 5; i++)
			plr->revival_init[i] = -1;

		RAssert(dylist_push(&server->game.players, plr));
	}

	Packet pack;
	PacketCreate(&pack, SERVER_LOBBY_GAME_START);
	server_broadcast(server, &pack);

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->in_game)
			continue;

		for (size_t j = 0; j < server->peers.capacity; j++)
		{
			PeerData* er = (PeerData*)server->peers.ptr[j];
			if (!er)
				continue;

			if (er->id == peer->id)
				continue;

			PacketCreate(&pack, SERVER_WAITING_PLAYER_INFO);
			PacketWrite(&pack, packet_write8, er->in_game);
			PacketWrite(&pack, packet_write16, er->id);
			PacketWrite(&pack, packet_writestr, er->nickname);

			if (er->in_game)
			{
				PacketWrite(&pack, packet_write8, server->game.exe == er->id);
				PacketWrite(&pack, packet_write8, server->game.exe == er->id ? er->exe_char : er->surv_char);
			}
			else
			{
				PacketWrite(&pack, packet_write8, er->lobby_icon);
			}

			RAssert(packet_sendtcp(server, peer->id, &pack));
		}
	}

	Info("Server is now in ST_GAME");
	return res;
}

bool game_uninit(Server* server)
{
	bool res = true;

	for (size_t i = 0; i < server->game.entities.capacity; i++)
	{
		Entity* entity = (Entity*)server->game.entities.ptr[i];
		if (!entity)
			continue;

		free(entity);
	}
	if (!dylist_free(&server->game.entities))
		res = false;

	for (size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* player = (Player*)server->game.players.ptr[i];
		if (!player)
			continue;

		free(player);
	}
	if (!dylist_free(&server->game.players))
		res = false;

	g_mapList[server->game.map].cb.uninit(server);
	RAssert(lobby_init(server));
	return res;
}

bool game_spawn(Server* server, Entity* entity, size_t len, Entity** out)
{
	entity->id = ++server->game.entid;
	Debug("Allocated entity \"%s\" (id %d, size %d)", entity->tag, entity->id, len);
	
	Entity* ent = (Entity*)malloc(len);
	if(!ent)
		return false;
	 
	memcpy(ent, entity, len);
	if (ent->init && !ent->init(server, ent))
	{
		free(ent);
		return false;
	}
	
	dylist_push(&server->game.entities, ent);
	if (out)
		*out = ent;
	return true;
}

bool game_despawn(Server* server, Entity** out, uint16_t id)
{
	bool res = true;
	Entity* tar = NULL;

	for (size_t i = 0; i < server->game.entities.capacity; i++)
	{
		Entity* entity = (Entity*)server->game.entities.ptr[i];
		if (!entity)
			continue;

		if (entity->id == id)
		{
			if (entity->uninit && !entity->uninit(server, entity))
				Warn("uninit failed for entity %d", entity->id);

			Debug("Deallocated entity \"%s\" (id %d)", entity->tag, entity->id);
			tar = entity;
			break;
		}
	}

	res = dylist_remove(&server->game.entities, tar);
	if (res && !out)
		free(tar);
	else if (out)
		*out = tar;
	else
		Warn("Failed to find entity %d", id);

	return res;
}

int game_find(Server* server, Entity** out, char* tag, size_t count)
{
	int i = 0;

	for (size_t it = 0; it < server->game.entities.capacity; it++)
	{
		Entity* entity = (Entity*)server->game.entities.ptr[it];
		if (!entity)
			continue;

		Debug("Search %s (id %d) vs %s", entity->tag, entity->id, tag);
		if (strcmp(entity->tag, tag) == 0)
		{
			if (!out)
				i++;
			else
				out[i++] = entity;
		}

		if ((size_t)i >= count)
			break;
	}

	Debug("Search for \"%s\" found %d entities", tag, i);
	return i;
}

Player* game_findplr(Server* server, uint16_t id)
{
	for (size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* plr = (Player*)server->game.players.ptr[i];
		if (!plr)
			continue;

		if (plr->id == id)
			return plr;
	}

	return NULL;
}

bool game_bigring(Server* server, BigRingState state)
{
	RAssert(server);

	if (server->game.bring_state == state)
		return true;

	Packet packet;
	switch (state)
	{
		case BS_NONE:
			break;

		case BS_DEACTIVATED:
		{
			Info("Big ring is deactivated!");

			PacketCreate(&packet, SERVER_GAME_SPAWN_RING);
			PacketWrite(&packet, packet_write8, 0);
			PacketWrite(&packet, packet_write8, server->game.bring_loc);
			server_broadcast(server, &packet);
			break;
		}

		case BS_ACTIVATED:
		{
			Info("Big ring is activated!");

			PacketCreate(&packet, SERVER_GAME_SPAWN_RING);
			PacketWrite(&packet, packet_write8, 1);
			PacketWrite(&packet, packet_write8, server->game.bring_loc);
			server_broadcast(server, &packet);
			break;
		}
	}

	server->game.bring_state = state;
	return true;
}

bool game_checkstart(Server* server)
{
	if (server->game.started)
		return true;

	uint8_t cnt = 0;
	for (size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* plr = (Player*)server->game.players.ptr[i];
		if (!plr)
			continue;

		if (plr->ready)
			cnt++;
	}

	if (cnt >= server_ingame(server))
	{
		Packet pack;
		PacketCreate(&pack, SERVER_GAME_PLAYERS_READY);
		server_broadcast(server, &pack);

		srand(time(NULL));
		RAssert(g_mapList[server->game.map].cb.init(server));
		Info("Game started! (Time %ds)", server->game.time_sec);

		server->game.started = 1;
	}

	return true;
}

bool game_demonize(Server* server, Player* player)
{
	Packet pack;
	PacketCreate(&pack, SERVER_GAME_DEATHTIMER_END);

	int players = 0;
	int demonized = 0;

	for (size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* player = (Player*)server->game.players.ptr[i];
		if (!player)
			continue;

		if (player->id == server->game.exe)
			continue;
		
		if (player->flags & PLAYER_DEMONIZED)
			demonized++;

		players++;
	}

	char name[128];
	uint8_t ch = 0;

	for(size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;

		if (peer->id == player->id)
		{
			strncpy(name, peer->nickname.value, 128);
			ch = peer->surv_char;
			break;
		}
	}

	if (players / 2 > demonized)
	{
		DEL_FLAG(player->flags, PLAYER_DEAD);
		SET_FLAG(player->flags, PLAYER_DEMONIZED);

		// reset cooldown
		switch (ch)
		{
			case CH_TAILS:
			{
				server->game.cooldowns[TAILS_RECHARGE] = 0;
				break;
			}

			case CH_EGGMAN:
			{
				server->game.cooldowns[EGGTRACK_RECHARGE] = 0;
				break;
			}

			case CH_CREAM:
			{
				server->game.cooldowns[CREAM_RING_SPAWN] = 0;
				break;
			}
		}

		Info("%s (id %d) was demonized!", name, player->id);
		PacketWrite(&pack, packet_write8, 1);
	}
	else
	{
		SET_FLAG(player->flags, PLAYER_CANTREVIVE);

		Info("%s (id %d) died!", name, player->id);
		PacketWrite(&pack, packet_write8, 0);
	}

	RAssert(packet_sendtcp(server, player->id, &pack));
	return true;
}

void game_broadcast(Server* server, Packet* packet)
{
	for(size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* plr = (Player*)server->game.players.ptr[i];
		if (!plr)
			continue;

		packet_sendudp(server->udp.fd, &plr->addr, packet);
	}
}

void game_broadcast_ex(Server* server, Packet* packet, IpAddr* except)
{
	for (size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* plr = (Player*)server->game.players.ptr[i];
		if (!plr)
			continue;

		if (except->v4.sin_addr.s_addr == plr->addr.v4.sin_addr.s_addr && except->v4.sin_port == plr->addr.v4.sin_port)
			continue;

		packet_sendudp(server->udp.fd, &plr->addr, packet);
	}
}

int8_t game_map(Server* server)
{
	return server->game.map;
}

double game_time(Server* server)
{
	return server->game.time;
}

uint16_t game_timesec(Server* server)
{
	return server->game.time_sec;
}

bool game_state_join(PeerData* v)
{
	(void)v; // dont fucking remember why this exists ~~
	return true;
}

bool game_state_left(PeerData* v)
{
	if (v->server->game.end > 0)
		return true;

	if (!v->in_game)
		return true;

	RAssert(g_mapList[v->server->game.map].cb.left(v));

	// Remove player from the list
	for (size_t i = 0; i < v->server->game.players.capacity; i++)
	{
		Player* plr = (Player*)v->server->game.players.ptr[i];
		if (!plr)
			continue;

		if (plr->id == v->id)
		{
			dylist_remove(&v->server->game.players, plr);
			free(plr);
			break;
		}
	}

	if (!v->server->game.started)
	{
		if (v->id == v->server->game.exe)
			return game_uninit(v->server);

		return game_checkstart(v->server);
	}

	if(server_ingame(v->server) <= 1)
		return game_uninit(v->server);

	if (v->id == v->server->game.exe)
		return game_end(v->server, ED_SURVWIN);

	RAssert(game_state_check(v->server));
	return true;
}

bool game_state_handletcp(PeerData* v, Packet* packet)
{
	// Read header
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);
	
	switch (type)
	{
		default:
			RAssert(server_msg_handle(v->server, type, v, packet));
			break;
			
		case CLIENT_PLAYER_POTATER:
		case CLIENT_SOUND_EMIT:
		case CLIENT_SPAWN_EFFECT:
		case CLIENT_PLAYER_PALLETE:
		case CLIENT_PET_PALLETE:
		case CLIENT_SPRING_USE:
		case CLIENT_PLAYER_HEAL:
		case CLIENT_PLAYER_HEAL_PART:
		case CLIENT_MERCOIN_BONUS:
		{
			server_broadcast_ex(v->server, packet, v->id);
			break;
		}

		case CLIENT_TPROJECTILE_STARTCHARGE:
		{
			time_start(&v->server->game.tails_last_proj);
			break;
		}

		case CLIENT_TPROJECTILE:
		{
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			AssertOrDisconnect(v->server, v->surv_char == CH_TAILS);
			AssertOrDisconnect(v->server, v->server->game.cooldowns[TAILS_RECHARGE] <= 0);
			AssertOrDisconnect(v->server, game_find(v->server, NULL, "tproj", 0) <= 0);

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(dir, packet, packet_read8, int8_t);
			PacketRead(dmg, packet, packet_read8, uint8_t);
			PacketRead(exe, packet, packet_read8, uint8_t);
			PacketRead(chg, packet, packet_read8, uint8_t);
			AssertOrDisconnect(v->server, dir >= -1 && dir <= 1);

			Player* plr = game_findplr(v->server, v->id);
			AssertOrDisconnect(v->server, plr);

			uint8_t check_balls = 1;
			if (plr->flags & PLAYER_DEMONIZED)
			{
				AssertOrDisconnect(v->server, dmg <= 60);

				if (dmg >= 60)
				{
					double last = time_end(&v->server->game.tails_last_proj);
					check_balls = last > 1500 && last < 12000;
					AssertOrDisconnect(v->server, check_balls);
				}
			}
			else
			{
				AssertOrDisconnect(v->server, dmg <= 6);

				if (dmg >= 6)
				{
					double last = time_end(&v->server->game.tails_last_proj);
					check_balls = last > 1500 && last < 12000;
					AssertOrDisconnect(v->server, check_balls);
				}
			}

			if (!check_balls)
				break;

			game_spawn(v->server, (Entity*)&(MakeTailsProj(x, y, v->id, dir, (plr->flags & PLAYER_DEMONIZED) > 0, chg, dmg)), sizeof(TProjectile), NULL);
			v->server->game.cooldowns[TAILS_RECHARGE] = 10 * TICKSPERSEC;
			break;
		}

		case CLIENT_TPROJECTILE_HIT:
		{
			Entity* ents;
			if (game_find(v->server, &ents, "tproj", 1))
				game_despawn(v->server, NULL, ents->id);

			break;
		}

		case CLIENT_RING_COLLECTED:
		{
			PacketRead(id, packet, packet_read8, uint8_t);
			PacketRead(eid, packet, packet_read16, uint16_t);

			Ring* ent = NULL;
			bool res = game_despawn(v->server, (Entity**)&ent, eid);
			if (res)
			{
				Packet pack;
				PacketCreate(&pack, SERVER_RING_COLLECTED);
				PacketWrite(&pack, packet_write8, ent->red);

				free(ent);
				RAssert(packet_sendtcp(v->server, v->id, &pack));
			}
			break;
		}

		case CLIENT_CREAM_SPAWN_RINGS:
		{
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			AssertOrDisconnect(v->server, v->surv_char == CH_CREAM);
			AssertOrDisconnect(v->server, v->server->game.cooldowns[CREAM_RING_SPAWN] <= 0);

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(red_ring, packet, packet_read8, uint8_t);

			#define PI 3.141593
			if (red_ring)
			{
				for (int i = 0; i < 2; i++)
				{
					uint16_t rX = x + sin(PI * 2.5 - (i * PI)) * 26 - 1;
					uint16_t rY = y + cos(PI * 2.5 - (i * PI)) * 26;

					RAssert(game_spawn(v->server, (Entity*)&(MakeCreamRing(rX, rY, red_ring)), sizeof(CreamRing), NULL));
				}
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					uint16_t rX = x + sin(PI * 2.5 + (i * (PI / 2))) * 26;
					uint16_t rY = y + cos(PI * 2.5 + (i * (PI / 2))) * 26;

					RAssert(game_spawn(v->server, (Entity*)&(MakeCreamRing(rX, rY, red_ring)), sizeof(CreamRing), NULL));
				}
			}

			v->server->game.cooldowns[CREAM_RING_SPAWN] = 25 * TICKSPERSEC;
			break;
		}

		case CLIENT_ETRACKER:
		{
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			AssertOrDisconnect(v->server, v->surv_char == CH_EGGMAN);
			AssertOrDisconnect(v->server, v->server->game.cooldowns[EGGTRACK_RECHARGE] <= 0);
			
			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			
			game_spawn(v->server, (Entity*)&(MakeEggTrack(x, y)), sizeof(EggTracker), NULL);
			v->server->game.cooldowns[EGGTRACK_RECHARGE] = 10 * TICKSPERSEC; 
			break;
		}

		case CLIENT_ETRACKER_ACTIVATED:
		{
			PacketRead(eid, packet, packet_read16, uint16_t);

			Entity* ents[48];
			int found = game_find(v->server, ents, "eggtrack", 47);

			Debug("Searching for tracker %d", eid);
			for (int i = 0; i < found; i++)
			{
				EggTracker* entity = (EggTracker*)ents[i];
				
				if (entity->id == eid)
				{
					entity->activ_id = v->id;
					Debug("Found tracker %d", entity->id);
					game_despawn(v->server, NULL, eid);
					break;
				}
			}

			break;
		}

		case CLIENT_ERECTOR_BRING_SPAWN:
		{
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);
			AssertOrDisconnect(v->server, v->exe_char == EX_EXETIOR);
			AssertOrDisconnect(v->server, v->server->game.cooldowns[EXETIOR_BRING_SPAWN] <= 0);

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);

			BRing* rings[128];
			int cnt = game_find(v->server, (Entity**)rings, "bring", 128);
			
			Vector2 pos = { x, y };
			for(int i = 0; i < cnt; i++)
			{
				BRing* ring = rings[i];
				
				AssertOrDisconnect(v->server, vector2_dist(&ring->pos, &pos) >= 100);
			}
			
			game_spawn(v->server, (Entity*)(&(MakeBlackRing(x, y))), sizeof(BRing), NULL);
			v->server->game.cooldowns[EXETIOR_BRING_SPAWN] = 10 * TICKSPERSEC;
			break;
		}

		case CLIENT_BRING_COLLECTED:
		{
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			PacketRead(eid, packet, packet_read16, uint16_t);

			if (game_despawn(v->server, NULL, eid))
			{
				Packet pack;
				PacketCreate(&pack, SERVER_BRING_COLLECTED);
				packet_sendtcp(v->server, v->id, &pack);
			}
			break;
		}

		case CLIENT_ERECTOR_BALLS:
		{
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);

			PacketRead(x, packet, packet_readfloat, float);
			PacketRead(y, packet, packet_readfloat, float);

			Packet pack;
			PacketCreate(&pack, CLIENT_ERECTOR_BALLS);
			PacketWrite(&pack, packet_writefloat, x);
			PacketWrite(&pack, packet_writefloat, y);
			server_broadcast(v->server, &pack);
			break;
		}

		case CLIENT_EXELLER_SPAWN_CLONE:
		{
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);
			AssertOrDisconnect(v->server, v->exe_char == EX_EXELLER);
			AssertOrDisconnect(v->server, game_find(v->server, NULL, "exclone", 0) < 2);

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(dir, packet, packet_read8, int8_t);
			game_spawn(v->server, (Entity*)&(MakeExellerClone(x, y, dir, v->id)), sizeof(ExellerClone), NULL);
			break;
		}

		case CLIENT_EXELLER_TELEPORT_CLONE:
		{
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);
			AssertOrDisconnect(v->server, v->exe_char == EX_EXELLER);
			
			PacketRead(eid, packet, packet_read16, uint16_t);
			Player* plr = game_findplr(v->server, v->id);
			if (!plr)
				break;

			if(game_despawn(v->server, NULL, eid))
				plr->ex_teleport = 60;

			break; 
		}

		case CLIENT_PLAYER_ESCAPED:
		{
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);

			Player* player = game_findplr(v->server, v->id);
			AssertOrDisconnect(v->server, player);

			if (player->flags & PLAYER_DEAD || player->flags & PLAYER_DEMONIZED)
				break;

			if (player->flags & PLAYER_ESCAPED)
				break;

			SET_FLAG(player->flags, PLAYER_ESCAPED);

			Packet pack;
			PacketCreate(&pack, SERVER_PLAYER_ESCAPED);
			RAssert(packet_sendtcp(v->server, v->id, &pack));

			PacketCreate(&pack, SERVER_GAME_PLAYER_ESCAPED);
			PacketWrite(&pack, packet_write16, v->id);
			server_broadcast(v->server, &pack);

			RAssert(game_state_check(v->server));
			break;
		}

		case CLIENT_PLAYER_DEATH_STATE:
		{
			if (v->server->game.end > 0)
				break;

			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			
			Player* player = game_findplr(v->server, v->id);
			AssertOrDisconnect(v->server, player);
			AssertOrDisconnect(v->server, !(player->flags & PLAYER_DEMONIZED));

			PacketRead(dead, packet, packet_read8, uint8_t);
			PacketRead(rtimes, packet, packet_read8, uint8_t);

			Packet pack;
			PacketCreate(&pack, SERVER_PLAYER_DEATH_STATE);
			PacketWrite(&pack, packet_write16, v->id);
			PacketWrite(&pack, packet_write8, dead);
			PacketWrite(&pack, packet_write8, rtimes);
			server_broadcast(v->server, &pack);

			PacketCreate(&pack, SERVER_REVIVAL_STATUS);
			PacketWrite(&pack, packet_write8, false)
			PacketWrite(&pack, packet_write16, v->id);
			server_broadcast(v->server, &pack);

			if (dead)
			{
				if (player->flags & PLAYER_DEAD || player->flags & PLAYER_ESCAPED)
					break;

				SET_FLAG(player->flags, PLAYER_DEAD);
				
				if (player->flags & PLAYER_REVIVED || v->server->game.time_sec < TICKSPERSEC * 2)
				{
					RAssert(game_demonize(v->server, player));
				}
				else
					player->death_timer_sec = 30;
			}
			else
			{
				AssertOrDisconnect(v->server, player->death_timer_sec > 0);
				DEL_FLAG(player->flags, PLAYER_DEAD);
			}

			RAssert(game_state_check(v->server));
			break;
		}

		case CLIENT_REVIVAL_PROGRESS:
		{
			if (v->server->game.end > 0)
				break;

			PacketRead(pid, packet, packet_read16, uint16_t);
			PacketRead(rings, packet, packet_read8, uint8_t);

			Player* revivor = game_findplr(v->server, v->id);
			Player* to_revive = game_findplr(v->server, pid);
			AssertOrDisconnect(v->server, revivor);

			if (!to_revive || !(to_revive->flags & PLAYER_DEAD))
				break;

			Packet pack;
			if (to_revive->flags & PLAYER_CANTREVIVE)
			{
				to_revive->death_timer_sec = 0;
				to_revive->death_timer = 0;

				PacketCreate(&pack, SERVER_REVIVAL_STATUS);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write16, to_revive->id);
				server_broadcast(v->server, &pack);
				break;
			}

			if (to_revive->revival <= 0)
			{
				PacketCreate(&pack, SERVER_REVIVAL_STATUS);
				PacketWrite(&pack, packet_write8, 1);
				PacketWrite(&pack, packet_write16, to_revive->id);
				server_broadcast(v->server, &pack);
			}

			to_revive->revival += 0.015 + (0.004 * rings);
			if (to_revive->revival < 1)
			{
				PacketCreate(&pack, SERVER_REVIVAL_PROGRESS);
				PacketWrite(&pack, packet_write16, to_revive->id);
				PacketWrite(&pack, packet_writedouble, to_revive->revival);
				game_broadcast(v->server, &pack);

				// add itself to the list
				{
					uint8_t has = 0;
					int ind = 0;

					for (int i = 0; i < 5; i++)
					{
						if (to_revive->revival_init[i] == -1)
						{
							ind = i;
							break;
						}

						if (to_revive->revival_init[i] == v->id)
							has = 1;
					}

					if (!has)
						to_revive->revival_init[ind] = v->id;
				}
			}
			else
			{
				SET_FLAG(to_revive->flags, PLAYER_REVIVED);
				DEL_FLAG(to_revive->flags, PLAYER_DEAD);

				PacketCreate(&pack, SERVER_REVIVAL_STATUS);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write16, to_revive->id);
				server_broadcast(v->server, &pack);

				PacketCreate(&pack, SERVER_REVIVAL_REVIVED);
				packet_sendtcp(v->server, to_revive->id, &pack);

				for (int i = 0; i < 5; i++)
				{
					if (to_revive->revival_init[i] == -1)
						break;

					PacketCreate(&pack, SERVER_REVIVAL_RINGSUB);
					packet_sendtcp(v->server, to_revive->revival_init[i], &pack);

					Debug("Removed rings from %d", to_revive->revival_init[i]);
				}

				// Print player's name
				for (size_t i = 0; i < v->server->peers.capacity; i++)
				{
					PeerData* peer = (PeerData*)v->server->peers.ptr[i];
					if (!peer)
						continue;

					if (peer->id == to_revive->id)
					{
						Info("%s (id %d) was revived!", peer->nickname.value, to_revive->id);
						break;
					}
				}
			}
			break;
		}

		case CLIENT_CHAT_MESSAGE:
		{
			if (v->in_game)
				break;

			PacketRead(pid, packet, packet_read16, uint16_t);
			PacketRead(msg, packet, packet_readstr, String);
			AssertOrDisconnect(v->server, string_length(&msg) <= 40);

			v->timeout = 0;

			Info("[%s] (id %d): %s", v->nickname.value, v->id, msg.value);
			if (!server_cmd_handle(v->server, server_cmd_parse(&msg), v, &msg))
				server_broadcast_ex(v->server, packet, v->id);

			break;
		}
	}

	if (!v->server->game.started)
		return true;

	RAssert(packet_seek(packet, 0));
	RAssert(g_mapList[v->server->game.map].cb.tcp_msg(v, packet));
	return true;
}

bool game_state_handleudp(Server* server, IpAddr* addr, Packet* packet)
{
	// Read header
	PacketRead(pid, packet, packet_read16, uint16_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	Packet pack;
	switch (type)
	{
		case CLIENT_PING:
		{
			if (!server->game.started)
				break;

			Player* plr = game_findplr(server, pid);
			if (!plr)
			{
				server_disconnect(server, pid, DR_PACKETSNOTRECV, "udp cancer");
				Warn("UDP cancer detected");
				return true;
			}

			PacketRead(ping, packet, packet_read64, uint64_t);
			PacketRead(calc, packet, packet_read16, uint16_t);

			PacketCreate(&pack, SERVER_PONG);
			PacketWrite(&pack, packet_write64, ping);
			packet_sendudp(server->udp.fd, addr, &pack);

			PacketCreate(&pack, SERVER_GAME_PING);
			PacketWrite(&pack, packet_write16, pid);
			PacketWrite(&pack, packet_write16, calc);
			game_broadcast_ex(server, &pack, addr);
			break;
		}

		case CLIENT_PLAYER_DATA:
		{
			if (!server->game.started)
				break;

			Player* player = game_findplr(server, pid);
			if (!player)
				break;

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			
			if (server->game.exe != player->id)
			{
				PacketRead(hp, packet, packet_read8, int8_t);
				if (hp > 100)
				{
					server_disconnect(server, player->id, DR_OTHER, "severe brain problem detected");
					return true;
				}
			}

			Vector2 new_pos = { x, y };
			
			// Calc distance before setting new pos
			float dist = vector2_dist(&player->pos, &new_pos);
			if (server->game.started && player->pos.x != 0 && player->pos.y != 0)
			{
				if (player->ex_teleport == 0)
				{
					if (dist > 700)
					{
						switch (server->game.map)
						{
							case 15:
							case 13:
							case 8:
							case 6:
								break;
			
							default:
								server_disconnect(server, player->id, DR_OTHER, "blud used a portal gun lmfao");
								return true;
						}
					}
			
					if (dist > 60)
					{
						if (!player_add_error(server, player, 500))
							return true;
					}
				}
				else
					player->ex_teleport--;
			}
			
			player->pos = new_pos;
			player->timeout = 0;

			PacketCreate(&pack, CLIENT_PLAYER_DATA);
			PacketWrite(&pack, packet_write16, pid);

			RAssert(packet_seek(packet, 3));
			while (packet->pos < packet->len)
			{
				PacketRead(i, packet, packet_read8, uint8_t);
				PacketWrite(&pack, packet_write8, i);
			}

			game_broadcast_ex(server, &pack, addr);
			break;
		}
	}

	if (!server->game.started)
	{
		for (size_t i = 0; i < server->game.players.capacity; i++)
		{
			Player* plr = (Player*)server->game.players.ptr[i];
			if (!plr)
				continue;

			if (plr->id == pid)
			{
				if (plr->ready)
					break;
				
				Debug("Accepted!");
				plr->ready = 1;
				memcpy(&plr->addr, addr, sizeof(IpAddr));
				break;
			}
		}

		RAssert(game_checkstart(server));
	}

	return true;
}

bool game_entity_tick(Server* server)
{
	int32_t entits[100];
	int entit = 0;
	memset(entits, -1, 100);
	for (size_t i = 0; i < server->game.entities.capacity; i++)
	{
		Entity* ent = (Entity*)server->game.entities.ptr[i];
		if (!ent)
			continue;

		if (ent->tick && !ent->tick(server, ent))
			entits[entit++] = ent->id;
	}

	for (int i = 0; i < 100; i++)
	{
		if (entits[i] == -1)
			break;

		game_despawn(server, NULL, (uint16_t)entits[i]);
	}

	return true;
}

int32_t game_cmptimer(const void* a, const void* b) 
{
	const Player* aa = (const Player*)a;
	const Player* bb = (const Player*)b;

	return (int32_t)aa->death_timer_sec - (int32_t)bb->death_timer_sec;
}

bool game_player_tick(Server* server)
{
	// Update player cooldown
	for (int i = 0; i < PLAYER_COOLCOUNT; i++)
	{
		if (server->game.cooldowns[i] > 0)
			server->game.cooldowns[i] -= server->delta;
	}
	
	int survivors = 0, demonized = 0;
	for(size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* player = (Player*)server->game.players.ptr[i];
		if (!player)
			continue;

		// subpussy revival time
		if(player->flags & PLAYER_DEAD && !(player->flags & PLAYER_CANTREVIVE))
		{
			if(player->revival > 0)
			{
				player->revival -= 0.0025 * server->delta;
				
				Packet pack;
				if(player->revival <= 0)
				{
					for(int i = 0; i < 5; i++)
						player->revival_init[i] = -1;

					PacketCreate(&pack, SERVER_REVIVAL_STATUS);
					PacketWrite(&pack, packet_write8, 0);
					PacketWrite(&pack, packet_write16, player->id);
					server_broadcast(server, &pack);
				}
				else
				{
					PacketCreate(&pack, SERVER_REVIVAL_PROGRESS);
					PacketWrite(&pack, packet_write16, player->id);
					PacketWrite(&pack, packet_writedouble, player->revival);
					game_broadcast(server, &pack);
				}
			}
		}

		if (!(player->flags & PLAYER_ESCAPED))
		{
			player->timeout += server->delta;
			if (player->timeout >= 4.f * TICKSPERSEC)
				server_disconnect(server, player->id, DR_AFKTIMEOUT, NULL);
			continue;
		}

		if (player->id == server->game.exe)
			continue;

		if (player->flags & PLAYER_DEMONIZED)
			demonized++;
		else if (!(player->flags & PLAYER_DEAD))
			survivors++;
	}

	// Start demonization
	if (!server->game.sudden_death && server->game.time_sec <= TICKSPERSEC * 2)
	{
		Player* sort[6];
		int len = 0;
		memset(sort, 0, sizeof(Player*) * 6);

		for (size_t i = 0; i < server->game.players.capacity; i++)
		{
			Player* player = (Player*)server->game.players.ptr[i];
			if (!player)
				continue;
			
			if (!(player->flags & PLAYER_DEAD))
				continue;

			sort[len++] = player;
		}

		for (int i = 0; i < len; ++i)
		{
			for (int j = i + 1; j < len; ++j)
			{
				float totalA = sort[i]->death_timer_sec + (sort[i]->death_timer / 60.0);
				float totalB = sort[j]->death_timer_sec + (sort[j]->death_timer / 60.0);
				
				if (totalA > totalB)
				{
					Player* a = sort[i];
					sort[i] = sort[j];
					sort[j] = a;
				}
			}
		}

		Info("Demonization order:");

		for (int i = 0; i < len; i++)
		{
			Info("%d: %f", sort[i]->id, sort[i]->death_timer_sec + (sort[i]->death_timer / 60.0));
			game_demonize(server, sort[i]);
		}
		server->game.sudden_death = 1;
	}

	for (size_t i = 0; i < server->game.players.capacity; i++)
	{
		Player* player = (Player*)server->game.players.ptr[i];
		if (!player)
			continue;

		if (player->flags & PLAYER_CANTREVIVE)
			continue;

		if (player->flags & PLAYER_DEAD && player->death_timer_sec > 0)
		{
			Packet packet;
			uint8_t exe_near = 0;

			// check if exe is nearby
			for (size_t j = 0; j < server->game.players.capacity; j++)
			{
				Player* check = (Player*)server->game.players.ptr[j];
				if (!check)
					continue;

				if (check->id != server->game.exe && !(check->flags & PLAYER_DEMONIZED))
					continue;

				if (vector2_dist(&player->pos, &check->pos) <= 240)
				{
					exe_near = 1;
					break;
				}
			}

			if (server->game.time_sec < TICKSPERSEC * 2)
			{
				game_demonize(server, player);
				continue;
			}

			if (player->death_timer >= TICKSPERSEC)
			{
				if (--player->death_timer_sec <= 0)
				{
					game_demonize(server, player);
					continue;
				}

				PacketCreate(&packet, SERVER_GAME_DEATHTIMER_TICK);
				PacketWrite(&packet, packet_write16, player->id);
				PacketWrite(&packet, packet_write8, player->death_timer_sec);
				server_broadcast(server, &packet);

				player->death_timer = 0;
			}

			if (exe_near)
				player->death_timer += 0.5f * server->delta;
			else
				player->death_timer += 1.0f * server->delta;
		}
	}

	return true;
}

bool game_state_tick(Server* server)
{
	// We wait 15 seconds, before kicking out everyone
	if (!server->game.started)
	{
		server->game.start_timeout -= server->delta;
		if (server->game.start_timeout <= 0)
		{
			Warn("Waiting for players took too long, kicking out inactive players!");

			for (size_t j = 0; j < server->game.players.capacity; j++)
			{
				Player* player = (Player*)server->game.players.ptr[j];

				if (!player)
					continue;

				if (!player->ready)
				{
					server_disconnect(server, player->id, DR_PACKETSNOTRECV, NULL);
					break;
				}
			}

		}
		return true;
	}

	if (server->game.end > 0)
	{
		server->game.end -= server->delta;
		if (server->game.end <= 0)
			RAssert(game_uninit(server));

		return true;
	}

	if (server->game.time <= TICKSPERSEC)
	{
		server->game.time_sec--;
		server->game.time += TICKSPERSEC;

		if (server->game.time_sec % server->game.ring_coff == 0)
		{
			if (!game_spawn(server, (Entity*)&(MakeRing()), sizeof(Ring), NULL))
				Warn("Failed to SHIT");
		}

		Packet packet;
		PacketCreate(&packet, SERVER_GAME_TIME_SYNC);
		PacketWrite(&packet, packet_write16, (uint16_t)server->game.time_sec * TICKSPERSEC);
		server_broadcast(server, &packet);

		if (server->game.time_sec <= 0)
			RAssert(game_end(server, ED_TIMEOVER));
	}

	RAssert(game_player_tick(server));
	RAssert(game_entity_tick(server));
	RAssert(g_mapList[server->game.map].cb.tick(server));

	server->game.time -= server->delta;
	return true;
}
