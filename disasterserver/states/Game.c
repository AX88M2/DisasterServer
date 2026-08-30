#include <io/Time.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <Player.h>
#include <States.h>
#include <CMath.h>
#include <DyList.h>
#include <Server.h>
#include <Palette.h>
#include <Colors.h>
#include <entities/Ring.h>
#include <entities/CreamRing.h>
#include <entities/BlackRing.h>
#include <entities/TailsProjectile.h>
#include <entities/EggmanTracker.h>
#include <entities/ExellerClone.h>

bool game_end(Server* server, Ending ending, bool achiv)
{
	if (server->game.end > 0)
		return true;

	Packet packet;
	switch (ending)
	{
		case ED_EXEWIN:
		{
			PacketCreate(&packet, SERVER_GAME_EXE_WINS);
			PacketWrite(&packet, packet_write8, achiv);
			Info("Ending is ED_EXEWIN");
			break;
		}

		case ED_SURVWIN:
		{
			PacketCreate(&packet, SERVER_GAME_SURVIVOR_WIN);
			PacketWrite(&packet, packet_write8, achiv);
			Info("Ending is ED_SURVWIN");
			break;
		}

		case ED_TIMEOVER:
		{
			PacketCreate(&packet, SERVER_GAME_TIME_OVER);
			PacketWrite(&packet, packet_write8, achiv);
			Info("Ending is ED_TIMEOVER");
			break;
		}
	}

	server_broadcast(server, &packet, true);
	server->game.end = 5 * TICKSPERSEC;
	server->game.ending = ending;
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
		for (size_t i = 0; i < server->peers.capacity; i++)
		{
			PeerData* v = (PeerData*)server->peers.ptr[i];
			if (!v)
				continue;

			if(!v->in_game)
				continue;

			if (v->plr.flags & PLAYER_ESCAPED)
				escaped++;

			if (v->plr.flags & PLAYER_DEAD || v->plr.flags & PLAYER_DEMONIZED)
				dead++;

			if (v->id == server->game.exe)
				exes++;
		}

		total = server_ingame(server);
		total -= (exes + dead + escaped);
	}

	if (total <= 0)
	{
		if (escaped > 0)
		{
			RAssert(game_end(server, ED_SURVWIN, true));
		}
		else
		{
			RAssert(game_end(server, ED_EXEWIN, true));
		}
	}

	return true;
}

bool game_init(int exe, int8_t map, Server* server)
{
	Debug("Attepting to enter ST_GAME...");
	RAssert(server_ingame(server) > 1);

	server->state = ST_GAME;
	server->game = (Game)
	{
		.map = map,
		.exe = exe,
		.bring_state = BS_NONE,
		.bring_loc = (uint8_t)rand(),
		.sudden_death = false,
		.started = false,
		.end = 0.0,
		.entid = 0,
		.time = TICKSPERSEC,
		.elapsed = 0.0,
		.start_timeout = 15.0 * TICKSPERSEC
	};

	// Setup cooldowns
	time_start(&server->game.tails_last_proj);
	memset(server->game.rings, 0, sizeof(server->game.rings));
	memset(server->game.cooldowns, 0, sizeof(server->game.cooldowns));

	if (!dylist_create(&server->game.entities, 3000))
		return false;
	Debug("Entity list created.");

	if (!dylist_create(&server->game.left, 7))
		return false;
	Debug("Left players list created.");

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* v = (PeerData*)server->peers.ptr[i];
		if (!v)
			continue;

		if (!v->in_game)
			continue;

		memset(&v->plr, 0, sizeof(Player));
		
		if (v->id == server->game.exe)
			SET_FLAG(v->plr.flags, PLAYER_KILLER);

		v->plr.ready = false;
		v->plr.mod_tool = v->mod_tool;

		for (int i = 0; i < 5; i++)
			v->plr.revival_init[i] = -1;
	}

	Packet pack;
	PacketCreate(&pack, SERVER_LOBBY_GAME_START);
	server_broadcast(server, &pack, true);

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* v = (PeerData*)server->peers.ptr[i];
		if (!v)
			continue;

		if (v->in_game)
			continue;

		for (size_t j = 0; j < server->peers.capacity; j++)
		{
			PeerData* er = (PeerData*)server->peers.ptr[j];
			if (!er)
				continue;

			if (er->id == v->id)
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

			RAssert(packet_send(v->peer, &pack, true));
		}
	}

	Info(LOG_YLW "Server is now in " LOG_PUR "Game");
	return true;
}

bool game_uninit(Server* server, bool show_results)
{
	if (show_results)
		return results_init(server);
	else
	{
		for (size_t i = 0; i < server->game.entities.capacity; i++)
		{
			Entity* entity = (Entity*)server->game.entities.ptr[i];
			if (!entity)
				continue;

			free(entity);
		}
		dylist_free(&server->game.entities);

		// Clean up after game
		for (size_t i = 0; i < server->game.left.capacity; i++)
		{
			PeerData* data = (PeerData*)server->game.left.ptr[i];
			if (!data)
				continue;

			free(data);
		}
		dylist_free(&server->game.left);

		return lobby_init(server);
	}
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

			if ((size_t)i >= count)
				break;
		}
	}

	Debug("Search for \"%s\" found %d entities", tag, i);
	return i;
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
			server_broadcast(server, &packet, true);
			break;
		}

		case BS_ACTIVATED:
		{
			Info("Big ring is activated!");

			PacketCreate(&packet, SERVER_GAME_SPAWN_RING);
			PacketWrite(&packet, packet_write8, 1);
			PacketWrite(&packet, packet_write8, server->game.bring_loc);
			server_broadcast(server, &packet, true);
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
	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* v = (PeerData*)server->peers.ptr[i];
		if (!v)
			continue;
		
		if(!v->in_game)
			continue;

		if (v->plr.ready)
			cnt++;
	}

	if (cnt >= server_ingame(server))
	{
		Packet pack;
		PacketCreate(&pack, SERVER_GAME_PLAYERS_READY);
		server_broadcast(server, &pack, true);

		srand((unsigned int)time(NULL));
		RAssert(g_mapList[server->game.map].cb.init(server));
		Info(LOG_YLW "Game started! " LOG_RST "(Time %ds)", server->game.time_sec);

		server->game.started = true;
	}

	return true;
}

bool game_demonize(Server* server, PeerData* data)
{
	Packet pack;
	PacketCreate(&pack, SERVER_GAME_DEATHTIMER_END);

	int players = 0;
	int demonized = 0;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* v = (PeerData*)server->peers.ptr[i];
		if (!v)
			continue;
		
		if(!v->in_game)
			continue;

		if (v->id == server->game.exe)
			continue;
		
		if (v->plr.flags & PLAYER_DEMONIZED)
			demonized++;

		players++;
	}

	if (players / 2 > demonized)
	{
		DEL_FLAG(data->plr.flags, PLAYER_DEAD);
		SET_FLAG(data->plr.flags, PLAYER_DEMONIZED);
		
		data->plr.stats.rings = 0;

		// reset cooldown
		switch (data->surv_char)
		{
			case CH_TAILS:
			{
				server->game.cooldowns[TAILS_RECHARGE] = 0;
				server->game.cooldowns[ETAILS_RECHARGE] = 0;
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

		Info("%s " LOG_RST "(id %d)" LOG_RED " was demonized!", data->nickname.value, data->id);
		PacketWrite(&pack, packet_write8, 1);
	}
	else
	{
		SET_FLAG(data->plr.flags, PLAYER_CANTREVIVE);

		Info("%s " LOG_RST "(id %d)" LOG_RED " died!", data->nickname.value, data->id);
		PacketWrite(&pack, packet_write8, 0);
	}

	RAssert(packet_send(data->peer, &pack, true));
	return true;
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

	if(server_ingame(v->server) <= 1)
		return game_uninit(v->server, false);

	if (!v->server->game.started)
	{
		if (v->id == v->server->game.exe)
			return game_uninit(v->server, false);

		return game_checkstart(v->server);
	}

	PeerData* data = (PeerData*)malloc(sizeof(PeerData));
	RAssert(data);
	memcpy(data, v, sizeof(PeerData));

	// Add player to the list
	SET_FLAG(data->plr.flags, PLAYER_LEFT);
	dylist_push(&v->server->game.left, data);

	if (v->id == v->server->game.exe)
		return game_end(v->server, ED_SURVWIN, v->server->game.elapsed >= TICKSPERSEC * TICKSPERSEC);

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
		case CLIENT_PET_PALETTE:
		case CLIENT_SPRING_USE:
		case CLIENT_MERCOIN_BONUS:
		case CLIENT_RING_BROKE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			server_broadcast_ex(v->server, packet, true,  v->id);
			break;
		}

		case CLIENT_PLAYER_PALETTE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, !g_config.anticheat || palette_player_validate(v, packet));
			server_broadcast_ex(v->server, packet, true,  v->id);
			break;
		}

		case CLIENT_PLAYER_HEAL_PART:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(rings, packet, packet_read16, uint16_t);
			v->plr.heal_rings = v->plr.rings;

			if (rings < 10)
			{
				server_disconnect(v->server, v->peer, DR_OTHER, "pusy");
				return true;
			}

			if (rings >= 140 && v->server->game.map != 20)
			{
				server_disconnect(v->server, v->peer, DR_OTHER, "dicus");
				return true;
			}

			server_broadcast_ex(v->server, packet, true, v->id);
			break;
		}

		case CLIENT_PLAYER_HEAL:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(id, packet, packet_read16, uint16_t);
			PacketRead(rings, packet, packet_read16, uint16_t);

			if (rings < 10)
			{
				server_disconnect(v->server, v->peer, DR_OTHER, "pusy");
				return true;
			}

			if (rings >= 140 && v->server->game.map != 20)
			{
				server_disconnect(v->server, v->peer, DR_OTHER, "dicus");
				return true;
			}

			if (v->plr.mod_tool)
			{
				Packet pack;
				PacketCreate(&pack, SERVER_RING_COLLECTED);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write16, 0);
				PacketWrite(&pack, packet_write8, true);
				PacketWrite(&pack, packet_write8, false);
				RAssert(packet_send(v->peer, &pack, true));
				break;
			}

			v->plr.heal_rings = 0;
			v->plr.stats.hp_restored++;

			server_broadcast_ex(v->server, packet, true, v->id);
			break;
		}

		case CLIENT_STATS_REPORT:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(type, packet, packet_read8, uint8_t);

			switch (type)
			{
				case 0:
				{
					v->plr.stats.hp_restored++;
					break;
				}

				case 1:
				{
					PacketRead(recv, packet, packet_read16, uint16_t);
					PacketRead(dmgr, packet, packet_read16, uint16_t);
					PacketRead(sec, packet, packet_read8, uint8_t);

					PeerData* rec = server_find_peer(v->server, recv);
					PeerData* damager = server_find_peer(v->server, dmgr);
					if (!rec || !damager)
						break;

					rec->plr.stats.stun_time += sec;
					damager->plr.stats.stuns++;
					break;
				}

				case 2:
				{
					PacketRead(id, packet, packet_read16, uint16_t);
					PacketRead(dmg, packet, packet_read16, uint16_t);
					PacketRead(hp, packet, packet_read16, uint16_t);
					
					PeerData* data = server_find_peer(v->server, id);
					if (!data)
						break;

					if (hp <= 0)
						data->plr.stats.kills++;

					data->plr.stats.damage += dmg / 20;
					break;
				}

				case 3:
				{
					PacketRead(dmg, packet, packet_read8, uint8_t);
					v->plr.stats.damage_taken += dmg / 20;
					break;
				}
			}
			break;
		}

		case CLIENT_PLAYER_HURT:
		{
			AssertOrDisconnect(v->server, v->in_game);
			server_broadcast_ex(v->server, packet, true, v->id);
			break;
		}

		case CLIENT_TPROJECTILE_STARTCHARGE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			time_start(&v->server->game.tails_last_proj);
			break;
		}

		case CLIENT_TPROJECTILE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			AssertOrDisconnect(v->server, v->surv_char == CH_TAILS);
			AssertOrDisconnect(v->server, game_find(v->server, NULL, "tproj", 10) <= 2);

			int cooldown_id = v->plr.flags & PLAYER_DEMONIZED ? ETAILS_RECHARGE : TAILS_RECHARGE;
			if(v->server->game.cooldowns[cooldown_id] > 0)
			{
				char msg[256];
				snprintf(msg, 256, "is_exe: %d, cool_id: %s, remaining_cooldown: %f", v->plr.flags & PLAYER_DEMONIZED, cooldown_id == TAILS_RECHARGE ? "TAILS_RECHARGE" : "ETAILS_RECHARGE", v->server->game.cooldowns[cooldown_id]);
				RAssert(server_disconnect(v->server, v->peer, DR_OTHER, msg));
				return false;
			}

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(dir, packet, packet_read8, int8_t);
			PacketRead(dmg, packet, packet_read8, uint8_t);
			PacketRead(exe, packet, packet_read8, uint8_t);
			PacketRead(chg, packet, packet_read8, uint8_t);
			AssertOrDisconnect(v->server, dir >= -1 && dir <= 1);

			if (v->mod_tool)
			{
				if (dir > 0)
					dir = -1;
				else if (dir < 0)
					dir = 1;
			}

			bool check_balls = true;
			if (v->plr.flags & PLAYER_DEMONIZED)
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

			RAssert(game_spawn(v->server, (Entity*)&(MakeTailsProj(x, y, v->id, dir, v->plr.flags & PLAYER_DEMONIZED, chg, dmg)), sizeof(TProjectile), NULL));
			v->server->game.cooldowns[cooldown_id] = 10 * TICKSPERSEC;
			break;
		}

		case CLIENT_TPROJECTILE_HIT:
		{
			AssertOrDisconnect(v->server, v->in_game);

			Entity* ents;
			if (game_find(v->server, &ents, "tproj", 1))
				game_despawn(v->server, NULL, ents->id);

			break;
		}

		case CLIENT_RING_COLLECTED:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(id, packet, packet_read8, uint8_t);
			PacketRead(eid, packet, packet_read16, uint16_t);

			Ring* ent = NULL;
			bool res = game_despawn(v->server, (Entity**)&ent, eid);
			if (res)
			{
				PeerData* data = server_find_peer(v->server, v->id);
				RAssert(data);

				if (!ent->red)
				{
					time_start(&data->plr.last_rings);
					data->plr.rings++;
					data->plr.stats.rings++;
				}

				Packet pack;
				PacketCreate(&pack, SERVER_RING_COLLECTED);
				PacketWrite(&pack, packet_write8, id);
				PacketWrite(&pack, packet_write16, eid);
				PacketWrite(&pack, packet_write8, ent->red);
				PacketWrite(&pack, packet_write8, data->plr.rings > 0);

				free(ent);
				RAssert(packet_send(v->peer, &pack, true));
			}
			break;
		}

		case CLIENT_CREAM_SPAWN_RINGS:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			AssertOrDisconnect(v->server, v->surv_char == CH_CREAM);
			AssertOrDisconnect(v->server, v->server->game.cooldowns[CREAM_RING_SPAWN] <= 0);

			if (v->mod_tool)
			{
				Packet pack;
				PacketCreate(&pack, SERVER_RING_COLLECTED);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write16, 0);
				PacketWrite(&pack, packet_write8, true);
				PacketWrite(&pack, packet_write8, false);
				RAssert(packet_send(v->peer, &pack, true));
				break;
			}

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(red_ring, packet, packet_read8, uint8_t);

			Vector2 pos = { (float)x, (float)y };
			AssertOrDisconnect(v->server, vector2_dist(&pos, &v->plr.pos) <= 40);

			static double PI = 0.0;
			if(PI == 0.0) 
				PI = acos(-1);

			if (red_ring)
			{				
				CreamRing* rings[128];
				int cnt = game_find(v->server, (Entity**)rings, "cring", 128);
				
				for(int i = 0; i < cnt; i++)
				{
					CreamRing* ring = rings[i];
					if(!ring->red)
						continue;

					AssertOrDisconnect(v->server, vector2_dist(&ring->pos, &pos) >= 150);
				}
				
				uint16_t posX[2] = { 25, -27 };
				uint16_t posY[2] = { 0, 0 };
				for (int i = 0; i < 2; i++)
				{
					uint16_t rX = x + posX[i];
					uint16_t rY = y + posY[i];

					RAssert(game_spawn(v->server, (Entity*)&(MakeCreamRing(rX, rY, red_ring)), sizeof(CreamRing), NULL));
				}
			}
			else
			{
				uint16_t posX[3] = { 26, 0, -27 };
				uint16_t posY[3] = { 0, -26, 0 };
				for (int i = 0; i < 3; i++)
				{
					uint16_t rX = x + posX[i];
					uint16_t rY = y + posY[i];

					RAssert(game_spawn(v->server, (Entity*)&(MakeCreamRing(rX, rY, red_ring)), sizeof(CreamRing), NULL));
				}
			}

			v->server->game.cooldowns[CREAM_RING_SPAWN] = 25 * TICKSPERSEC;
			break;
		}

		case CLIENT_ETRACKER:
		{
			AssertOrDisconnect(v->server, v->in_game);
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
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(eid, packet, packet_read16, uint16_t);
			
			Entity* ents[50];
			int found = game_find(v->server, ents, "eggtrack", 50);

			for (int i = 0; i < found; i++)
			{
				EggTracker* entity = (EggTracker*)ents[i];
				
				if (entity->id == eid)
				{
					entity->activ_id = v->id;
					game_despawn(v->server, NULL, eid);
					break;
				}
			}

			break;
		}

		case CLIENT_ERECTOR_BRING_SPAWN:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);
			AssertOrDisconnect(v->server, v->exe_char == EX_EXETIOR);
			AssertOrDisconnect(v->server, v->server->game.cooldowns[EXETIOR_BRING_SPAWN] <= 0);

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);

			if (v->mod_tool)
			{
				RAssert(game_spawn(v->server, (Entity*)&(MakeCreamRing(x, y, false)), sizeof(CreamRing), NULL));
				break;
			}

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
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			PacketRead(eid, packet, packet_read16, uint16_t);

			if (game_despawn(v->server, NULL, eid))
			{
				Packet pack;
				PacketCreate(&pack, SERVER_BRING_COLLECTED);
				packet_send(v->peer, &pack, true);
			}
			break;
		}

		case CLIENT_ERECTOR_BALLS:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);

			PacketRead(x, packet, packet_readfloat, float);
			PacketRead(y, packet, packet_readfloat, float);

			if(!v->mod_tool) 
			{
				Packet pack;
				PacketCreate(&pack, CLIENT_ERECTOR_BALLS);
				PacketWrite(&pack, packet_writefloat, x);
				PacketWrite(&pack, packet_writefloat, y);
				server_broadcast(v->server, &pack, true);
			}
			else
			{
				for(int i = -3; i < 3; i++)
				{
					RAssert(game_spawn(v->server, (Entity*)&(MakeCreamRing(x + i * 8, y, false)), sizeof(CreamRing), NULL));
				}
			}

			break;
		}

		case CLIENT_EXELLER_SPAWN_CLONE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);
			AssertOrDisconnect(v->server, v->exe_char == EX_EXELLER);
			AssertOrDisconnect(v->server, game_find(v->server, NULL, "exclone", 3) < 2);

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(dir, packet, packet_read8, int8_t);
			game_spawn(v->server, (Entity*)&(MakeExellerClone(x, y, dir, v->id)), sizeof(ExellerClone), NULL);
			break;
		}

		case CLIENT_EXELLER_TELEPORT_CLONE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id == v->server->game.exe);
			AssertOrDisconnect(v->server, v->exe_char == EX_EXELLER);
			
			PacketRead(eid, packet, packet_read16, uint16_t);

			ExellerClone* clone;
			if (game_despawn(v->server, (Entity**)&clone, eid))
			{
				if (v->mod_tool)
				{
					Packet pack;
					PacketCreate(&pack, SERVER_EXELLERCLONE_STATE);
					PacketWrite(&pack, packet_write8, 0);
					PacketWrite(&pack, packet_write16, clone->id);
					PacketWrite(&pack, packet_write16, clone->owner);
					PacketWrite(&pack, packet_write16, (uint16_t)v->plr.pos.x);
					PacketWrite(&pack, packet_write16, (uint16_t)v->plr.pos.y);
					PacketWrite(&pack, packet_write8, clone->dir);
					RAssert(packet_send(v->peer, &pack, true));
					free(clone);
					break;
				}

				Packet pack;
				PacketCreate(&pack, SERVER_EXELLERCLONE_STATE);
				PacketWrite(&pack, packet_write8, 1);
				PacketWrite(&pack, packet_write16, clone->id);
				server_broadcast(v->server, &pack, true);

				free(clone);
				v->plr.ex_teleport = 60;
			}
			break; 
		}

		case CLIENT_PLAYER_ESCAPED:
		{
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);

			if (v->mod_tool)
			{
				RAssert(server_disconnect(v->server, v->peer, DR_SERVERTIMEOUT, NULL));
				break;
			}

			if (v->plr.flags & PLAYER_DEAD || v->plr.flags & PLAYER_DEMONIZED)
				break;

			if (v->plr.flags & PLAYER_ESCAPED)
				break;

			SET_FLAG(v->plr.flags, PLAYER_ESCAPED);

			Packet pack;
			PacketCreate(&pack, SERVER_PLAYER_ESCAPED);
			RAssert(packet_send(v->peer, &pack, true));

			PacketCreate(&pack, SERVER_GAME_PLAYER_ESCAPED);
			PacketWrite(&pack, packet_write16, v->id);
			server_broadcast(v->server, &pack, true);

			RAssert(game_state_check(v->server));
			break;
		}

		case CLIENT_PLAYER_DEATH_STATE:
		{
			if (v->server->game.end > 0)
				break;

			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, v->id != v->server->game.exe);
			AssertOrDisconnect(v->server, !(v->plr.flags & PLAYER_DEMONIZED));

			PacketRead(dead, packet, packet_read8, uint8_t);
			PacketRead(rtimes, packet, packet_read8, uint8_t);

			Packet pack;
			PacketCreate(&pack, SERVER_PLAYER_DEATH_STATE);
			PacketWrite(&pack, packet_write16, v->id);
			PacketWrite(&pack, packet_write8, dead);
			PacketWrite(&pack, packet_write8, rtimes);
			server_broadcast(v->server, &pack, true);

			PacketCreate(&pack, SERVER_REVIVAL_STATUS);
			PacketWrite(&pack, packet_write8, false)
			PacketWrite(&pack, packet_write16, v->id);
			server_broadcast(v->server, &pack, true);

			if (dead)
			{
				if (v->plr.flags & PLAYER_DEAD || v->plr.flags & PLAYER_ESCAPED)
					break;

				SET_FLAG(v->plr.flags, PLAYER_DEAD);
				if (v->plr.flags & PLAYER_REVIVED || v->server->game.time_sec < TICKSPERSEC * 2)
				{
					RAssert(game_demonize(v->server, v));
				}
				else
				{
					PeerData* exe = server_find_peer(v->server, v->server->game.exe);
					v->plr.death_timer_sec = 30;

					PacketCreate(&pack, SERVER_GAME_DEATHTIMER_TICK);
					PacketWrite(&pack, packet_write8, exe && vector2_dist(&v->plr.pos, &exe->plr.pos) <= 240);
					PacketWrite(&pack, packet_write16, v->id);
					PacketWrite(&pack, packet_write8, v->plr.death_timer_sec);
					server_broadcast(v->server, &pack, true);
				}
			}
			else
			{
				AssertOrDisconnect(v->server, v->plr.death_timer_sec > 0);
				DEL_FLAG(v->plr.flags, PLAYER_DEAD);
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

			PeerData* to_revive = server_find_peer(v->server, pid);
			AssertOrDisconnect(v->server, to_revive);

			if (!to_revive || !(to_revive->plr.flags & PLAYER_DEAD))
				break;

			Packet pack;
			if (to_revive->plr.flags & PLAYER_CANTREVIVE)
			{
				to_revive->plr.death_timer_sec = 0;
				to_revive->plr.death_timer = 0;

				PacketCreate(&pack, SERVER_REVIVAL_STATUS);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write16, to_revive->id);
				server_broadcast(v->server, &pack, true);
				break;
			}

			if (to_revive->plr.revival <= 0)
			{
				PacketCreate(&pack, SERVER_REVIVAL_STATUS);
				PacketWrite(&pack, packet_write8, 1);
				PacketWrite(&pack, packet_write16, to_revive->id);
				server_broadcast(v->server, &pack, true);
			}

			to_revive->plr.revival += 0.015 + (0.004 * rings);
			if (to_revive->plr.revival < 1)
			{
				PacketCreate(&pack, SERVER_REVIVAL_PROGRESS);
				PacketWrite(&pack, packet_write16, to_revive->id);
				PacketWrite(&pack, packet_writedouble, to_revive->plr.revival);
				server_broadcast(v->server, &pack, false);

				// add itself to the list
				{
					bool has = false;
					int ind = 0;

					for (int i = 0; i < 5; i++)
					{
						if (to_revive->plr.revival_init[i] == -1)
						{
							ind = i;
							break;
						}

						if (to_revive->plr.revival_init[i] == v->id)
							has = true;
					}

					if (!has)
						to_revive->plr.revival_init[ind] = v->id;
				}
			}
			else
			{
				to_revive->plr.stats.rings = 0;

				SET_FLAG(to_revive->plr.flags, PLAYER_REVIVED);
				DEL_FLAG(to_revive->plr.flags, PLAYER_DEAD);

				PacketCreate(&pack, SERVER_REVIVAL_STATUS);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write16, to_revive->id);
				server_broadcast(v->server, &pack, true);

				PacketCreate(&pack, SERVER_REVIVAL_REVIVED);
				packet_send(to_revive->peer, &pack, true);

				for (int i = 0; i < 5; i++)
				{
					if (to_revive->plr.revival_init[i] == -1)
						break;

					PeerData* data = server_find_peer(v->server, to_revive->plr.revival_init[i]);
					if(!data)
						continue;
					
					PacketCreate(&pack, SERVER_REVIVAL_RINGSUB);
					packet_send(data->peer, &pack, true);

					Debug("Removed rings from %d", to_revive->plr.revival_init[i]);
				}

				// Print player's name
				Info("%s " LOG_RST "(id %d)" LOG_GRN " was revived!", to_revive->nickname.value, to_revive->id);
			}
			break;
		}

		case CLIENT_CHAT_MESSAGE:
		{
			if (v->in_game)
				break;

			v->timeout = 0;

			PacketRead(pid, packet, packet_read16, uint16_t);
			PacketRead(msg, packet, packet_readstr, String);
			AssertOrDisconnect(v->server, string_length(&msg) <= 40);

			Info("%s " LOG_RST "(id %d): %s", v->nickname.value, v->id, msg.value);
			if (!server_cmd_handle(v->server, server_cmd_parse(&msg), v, &msg))
				server_broadcast_ex(v->server, packet, true, v->id);

			break;
		}
		
		case CLIENT_PING:
		{
			if (!v->server->game.started)
				break;

			if (v->plr.mod_tool)
			{
				if (v->server->game.time_sec <= TICKSPERSEC * 2 + 5)
					break;
			}

			Packet pack;
			PacketCreate(&pack, SERVER_PONG);
			PacketWrite(&pack, packet_write16, v->peer->roundTripTime);
			packet_send(v->peer, &pack, false);
			v->plr.ping_last = v->peer->roundTripTime;

			PacketCreate(&pack, SERVER_GAME_PING);
			PacketWrite(&pack, packet_write16, v->id);
			PacketWrite(&pack, packet_write16, v->peer->roundTripTime);
			server_broadcast_ex(v->server, &pack, false, v->id);
			break;
		}

		case CLIENT_PLAYER_DATA:
		{
			if (!v->server->game.started)
				break;

			PacketRead(x, packet, packet_read16, uint16_t);
			PacketRead(y, packet, packet_read16, uint16_t);
			PacketRead(_xspd, packet, packet_read16, uint16_t);
			PacketRead(_yspd, packet, packet_read16, uint16_t);
			
			PacketRead(state, packet, packet_read8, uint8_t);
			PacketRead(_angle, packet, packet_read16, int16_t);
			PacketRead(_index, packet, packet_read8, uint8_t);
			PacketRead(_xscale, packet, packet_read8, int8_t);

			Vector2 new_pos = { x, y };
			#define PLAYER_ATTACKING 1 << 4
			
			int duration = 2000;
			if(v->server->game.exe != v->id)
			{
				PacketRead(hp, packet, packet_read8, int8_t);
				PacketRead(revival, packet, packet_read8, uint8_t);
				PacketRead(rings, packet, packet_read16, int16_t);
				PacketRead(flags, packet, packet_read8, uint8_t);

				if(!(v->plr.flags & PLAYER_DEAD) && !(v->plr.flags & PLAYER_DEMONIZED))
				{
					if (v->server->game.exe != v->id)
					{
						v->plr.rings = rings;
						if (revival < 2)
						{
							if (rings < 0 || (v->server->game.map != 20 && rings >= 120))
							{
								server_disconnect(v->server, v->peer, DR_OTHER, "gomunkulus");
								return true;
							}

							if (hp > 100)
							{
								server_disconnect(v->server, v->peer, DR_OTHER, "garic forn — Сьогодні о 04:31");
								return true;
							}
						}
					}
				}

				v->plr.is_attacking = flags & PLAYER_ATTACKING;
				switch(v->surv_char)
				{
					case CH_EGGMAN:
					{
						duration = 3000;
						break;
					}
				}
			}
			else
			{
				PacketRead(flags, packet, packet_read8, uint8_t);
				v->plr.is_attacking = flags & PLAYER_ATTACKING;
			}

			if(v->server->game.end <= 0 && v->plr.is_attacking)
			{
				if(v->plr.attack_timer <= 0)
				{
					time_start(&v->plr.last_attack);
					v->plr.attack_timer = 1;
				}
				else
				{
					double elapsed = time_end(&v->plr.last_attack);
					v->plr.attack_timer += elapsed;
					if(v->plr.attack_timer >= duration)
					{
						server_disconnect(v->server, v->peer, DR_KICKEDBYHOST, "nuh-uh!");
						return true;
					}
					time_start(&v->plr.last_attack);
				}
			}
			else
				v->plr.attack_timer = 0;
			
			// ping limit disabled
			if(g_config.ping_limit == UINT16_MAX)
			{
				// Calc distance before setting new posx
				float dist = vector2_dist(&v->plr.pos, &new_pos);
				if (v->server->game.started && !v->plr.mod_tool && v->plr.pos.x != 0 && v->plr.pos.y != 0)
				{
					if (v->plr.ex_teleport == 0)
					{
						if (dist > 700)
						{
							switch (v->server->game.map)
							{
								case 15:
								case 13:
								case 8:
								case 6:
									break;
				
								default:
									server_disconnect(v->server, v->peer, DR_OTHER, "blud used a portal gun lmfao");
									return true;
							}
						}
				
						if (dist > 60)
						{
							if (!player_add_error(v->server, v, 500))
								return true;
						}
					}
					else
						v->plr.ex_teleport--;
				}
			}
			
			v->plr.pos = new_pos;
			v->plr.timeout = 0;

			if (v->plr.mod_tool)
			{
				if (v->plr.mod_tool_timer < 61)
					v->plr.mod_tool_timer++;
				else
				{
					float dist = vector2_dist(&v->plr.pos, &v->plr.start_pos);
					if (dist > 240)
					{
						Packet pack;
						PacketCreate(&pack, SERVER_PLAYER_BACKTRACK);
						PacketWrite(&pack, packet_writefloat, v->plr.start_pos.x);
						PacketWrite(&pack, packet_writefloat, v->plr.start_pos.y);
						packet_send(v->peer, &pack, true);
					}
					else if (dist >= 300)
					{
						// purpl men
						server_disconnect(v->server, v->peer, DR_OTHER,
							"@@@@@@@......@@@\n"
							"@@@@@@@......@@@\n"
							"@@@@@@@.......@@\n"
							"@@@@@@@@@@@@....\n"
							"@@@@@@@@@@......\n"
							"....@@..........\n"
							"................\n"
							"@@@@@@@@@@@@....\n"
							"@@@@@@@@@@@@....\n"
							"@@@@@@@@@@@@@...\n"
							"@@@@@@@@@@@@@...\n"
							"@@@@@@@@@@@....."
						);
					}
					break;
				}

				if (v->plr.mod_tool_timer == 60)
				{
					v->plr.start_pos = v->plr.pos;

					Packet pack;
					PacketCreate(&pack, SERVER_FELLA);
					PacketWrite(&pack, packet_write16, v->id);
					PacketWrite(&pack, packet_writefloat, v->plr.start_pos.x);
					PacketWrite(&pack, packet_writefloat, v->plr.start_pos.y);
					server_broadcast(v->server, &pack, true);
					break;
				}
			}

			if(v->plr.state != state || time_end(&v->plr.last_packet) >= 15 * 2.9)
			{
				v->plr.state = state;
				time_start(&v->plr.last_packet);

				Packet pack;
				PacketCreate(&pack, CLIENT_PLAYER_DATA);
				PacketWrite(&pack, packet_write16, v->id);

				RAssert(packet_seek(packet, 2));
				while (packet->pos < packet->len)
				{
					PacketRead(i, packet, packet_read8, uint8_t);
					PacketWrite(&pack, packet_write8, i);
				}

				server_broadcast_ex(v->server, &pack, false, v->id);
			}
			break;
		}
	}

	if (!v->server->game.started)
	{
		if(!v->plr.ready)
		{
			time_start(&v->plr.last_packet);
			v->plr.ready = true;
		}

		RAssert(game_checkstart(v->server));
		return true;
	}

	RAssert(packet_seek(packet, 0));
	RAssert(g_mapList[v->server->game.map].cb.tcp_msg(v, packet));
	return true;
}

bool game_entity_tick(Server* server)
{
	int32_t entits[100];
	int entit = 0;

	for(int i = 0; i < 100; i++)
		entits[i] = -1;

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

bool game_player_tick(Server* server)
{
	// Update player cooldown
	for (int i = 0; i < PLAYER_COOLCOUNT; i++)
	{
		if (server->game.cooldowns[i] > 0)
			server->game.cooldowns[i] -= server->delta;
		else
			server->game.cooldowns[i] = 0;
	}
	
	PeerData* exe = server_find_peer(server, server->game.exe);
	if(!exe)
		return true;

	bool exe_camp = false;
	int survivors = 0, demonized = 0;

	for(size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* data = (PeerData*)server->peers.ptr[i];
		if (!data)
			continue;

		if(!data->in_game)
			continue;

		player_check_zone(server, data);

		// ping check
		if (server->delta < 2.5)
		{
			data->plr.ping_timer += server->delta;
			data->plr.ping_total += data->plr.ping_last * server->delta;

			if (data->plr.ping_timer >= 20 * TICKSPERSEC)
			{
				double avg_ping = data->plr.ping_total / data->plr.ping_timer;
				if (avg_ping >= g_config.ping_limit)
				{
					char msg[130];
					snprintf(msg, 130, "Bad connection, try picking closest region for better experience!\nYour average ping for last 20s: %dms", (int)avg_ping);
					server_disconnect(server, data->peer, DR_OTHER, msg);
					continue;
				}

				data->plr.ping_total = 0;
				data->plr.ping_timer = 0;
			}
		}

		if((data->plr.flags & PLAYER_DEAD) && !(data->plr.flags & PLAYER_CANTREVIVE) && vector2_dist(&exe->plr.pos, &data->plr.pos) < 300)
			exe_camp = true;

		if (exe->id != data->id && !(data->plr.flags & PLAYER_DEAD) && !(data->plr.flags & PLAYER_DEMONIZED))
		{
			// calc danger time
			if (!(data->plr.flags & PLAYER_ESCAPED))
			{
				bool in_danger = exe && vector2_dist(&data->plr.pos, &exe->plr.pos) < 300;
				if (in_danger)
					data->plr.stats.danger_time += server->delta;
				
				if(server->game.map != 8 && server->game.map != 6)
				{
					// calc balls
					uint32_t chunk = ((uint32_t)data->plr.pos.x / 480) + ((uint32_t)data->plr.pos.y / 270);
					if (data->plr.chunk != chunk)
					{
						data->plr.stats.braindead_time = 0;
						data->plr.chunk = chunk;
					}
					else
					{
						data->plr.stats.braindead_time += server->delta * (in_danger ? 0.5 : 1);
						if (data->plr.stats.braindead_time >= 25 * TICKSPERSEC)
							data->plr.stats.brain_damage = true;
					}
				}
			}

			data->plr.stats.survive_time += server->delta;
		}
		
		// subpussy revival time
		if(data->plr.flags & PLAYER_DEAD && !(data->plr.flags & PLAYER_CANTREVIVE))
		{
			if(data->plr.revival > 0)
			{
				data->plr.revival -= 0.0025 * server->delta;
				
				Packet pack;
				if(data->plr.revival <= 0)
				{
					for(int i = 0; i < 5; i++)
						data->plr.revival_init[i] = -1;

					PacketCreate(&pack, SERVER_REVIVAL_STATUS);
					PacketWrite(&pack, packet_write8, 0);
					PacketWrite(&pack, packet_write16, data->id);
					server_broadcast(server, &pack, true);
				}
				else
				{
					PacketCreate(&pack, SERVER_REVIVAL_PROGRESS);
					PacketWrite(&pack, packet_write16, data->id);
					PacketWrite(&pack, packet_writedouble, data->plr.revival);
					server_broadcast(server, &pack, false);
				}
			}
		}

		if (!(data->plr.flags & PLAYER_ESCAPED))
		{
			data->plr.timeout += server->delta;
			if (data->plr.timeout >= 4.f * TICKSPERSEC)
				server_disconnect(server, data->peer, DR_AFKTIMEOUT, NULL);
			continue;
		}

		if (data->id == server->game.exe)
			continue;

		if (data->plr.flags & PLAYER_DEMONIZED)
			demonized++;
		else if (!(data->plr.flags & PLAYER_DEAD))
			survivors++;
	}

	if(server->game.map != 8 && exe_camp)
		exe->plr.stats.camp_time += server->delta;

	// Start demonization
	if (!server->game.sudden_death && server->game.time_sec <= TICKSPERSEC * 2)
	{
		server->game.sudden_death = true;

		PeerData* sort[6];
		int len = 0;
		memset(sort, 0, sizeof(Player*) * 6);

		for (size_t i = 0; i < server->peers.capacity; i++)
		{
			PeerData* data = (PeerData*)server->peers.ptr[i];
			if (!data)
				continue;

			if(!data->in_game)
				continue;

			if (!(data->plr.flags & PLAYER_DEAD))
				continue;

			sort[len++] = data;
		}

		for (int i = 0; i < len; ++i)
		{
			for (int j = i + 1; j < len; ++j)
			{
				double totalA = sort[i]->plr.death_timer_sec + (sort[i]->plr.death_timer / 60.0);
				double totalB = sort[j]->plr.death_timer_sec + (sort[j]->plr.death_timer / 60.0);
				
				if (totalA > totalB)
				{
					PeerData* a = sort[i];
					sort[i] = sort[j];
					sort[j] = a;
				}
			}
		}

		Debug("Demonization order:");
		for (int i = 0; i < len; i++)
		{
			Debug("%d: %f", sort[i]->id, sort[i]->plr.death_timer_sec + (sort[i]->plr.death_timer / 60.0));
			game_demonize(server, sort[i]);
		}
	}

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* data = (PeerData*)server->peers.ptr[i];
		if (!data)
			continue;

		if(!data->in_game)
			continue;

		if (data->plr.flags & PLAYER_CANTREVIVE)
			continue;

		if (data->plr.flags & PLAYER_DEAD && data->plr.death_timer_sec > 0)
		{
			Packet packet;
			bool exe_near = false;
			bool demonized_near = false;

			// check if exe is nearby
			for (size_t j = 0; j < server->peers.capacity; j++)
			{
				PeerData* check = (PeerData*)server->peers.ptr[j];
				if (!check)
					continue;

				if(!check->in_game)
					continue;

				if (check->id != server->game.exe && !(check->plr.flags & PLAYER_DEMONIZED))
					continue;

				if (vector2_dist(&data->plr.pos, &check->plr.pos) <= 240)
				{
					if (check->plr.flags & PLAYER_DEMONIZED)
						demonized_near = true;
					else
					{
						demonized_near = false;
						exe_near = true;
						break;
					}
				}
			}

			if (server->game.time_sec < TICKSPERSEC * 2)
			{
				game_demonize(server, data);
				continue;
			}

			if (data->plr.death_timer >= TICKSPERSEC)
			{
				if (!exe_near)
				{
					if (--data->plr.death_timer_sec <= 0)
					{
						game_demonize(server, data);
						continue;
					}
				}

				PacketCreate(&packet, SERVER_GAME_DEATHTIMER_TICK);
				PacketWrite(&packet, packet_write8, exe_near);
				PacketWrite(&packet, packet_write16, data->id);
				PacketWrite(&packet, packet_write8, data->plr.death_timer_sec);
				server_broadcast(server, &packet, true);

				data->plr.death_timer = 0;
			}

			data->plr.death_timer += (demonized_near ? 0.5f : 1.0f) * server->delta;
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

			for (size_t j = 0; j < server->peers.capacity; j++)
			{
				PeerData* data = (PeerData*)server->peers.ptr[j];
				if (!data)
					continue;

				if(!data->in_game)
					continue;

				if (!data->plr.ready)
				{
					server_disconnect(server, data->peer, DR_PACKETSNOTRECV, NULL);
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
			RAssert(game_uninit(server, true));

		return true;
	}

	server->game.elapsed += server->delta;
	if (server->game.time <= TICKSPERSEC)
	{
		server->game.time_sec--;
		server->game.time += TICKSPERSEC;

		if (server->game.time_sec % server->game.ring_coff == 0)
		{
			if (!game_spawn(server, (Entity*)&(MakeRing()), sizeof(Ring), NULL))
				Debug("Not enough space for rings");
		}

		Packet packet;
		PacketCreate(&packet, SERVER_GAME_TIME_SYNC);
		PacketWrite(&packet, packet_write16, (uint16_t)server->game.time_sec * TICKSPERSEC);
		server_broadcast(server, &packet, true);

		if (server->game.time_sec <= 0)
			RAssert(game_end(server, ED_TIMEOVER, true));
	}

	RAssert(game_player_tick(server));
	RAssert(game_entity_tick(server));
	RAssert(g_mapList[server->game.map].cb.tick(server));

	server->game.time -= server->delta;
	return true;
}
