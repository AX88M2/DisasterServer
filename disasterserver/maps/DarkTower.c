#include <maps/DarkTower.h>
#include <entities/DTBall.h>
#include <entities/DTStalactits.h>
#include <entities/TailsDoll.h>

bool dt_init(Server* server)
{
	RAssert(map_time(server, 3.42 * TICKSPERSEC, 20)); //205
	RAssert(map_ring(server, 5));

	RAssert(game_spawn(server, (Entity*)&(MakeDTBall()), sizeof(DTBall), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeTailsDoll()), sizeof(TailsDoll), NULL));

	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(0, 1744, 224)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(1, 1840, 224)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(2, 1936, 224)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(3, 2032, 224)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(4, 2128, 224)), sizeof(DTStalactits), NULL));
	
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(5, 1824, 784)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(6, 1920, 784)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(7, 2016, 784)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(8, 2112, 784)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(9, 2208, 784)), sizeof(DTStalactits), NULL));
	
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(10, 2464, 1384)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(11, 2592, 1384)), sizeof(DTStalactits), NULL));
	
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(12, 3032, 64)), sizeof(DTStalactits), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTStalactiti(13, 3088, 64)), sizeof(DTStalactits), NULL));

	return true;
}

bool dt_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_DTASS_ACTIVATE:
		{
			PacketRead(sid, packet, packet_read8, uint8_t);
			AssertOrDisconnect(v->server, v->in_game);
			AssertOrDisconnect(v->server, sid < 14);

			DTStalactits* ents[14];
			if (!game_find(v->server, (Entity**)ents, "dttits", 14))
				break;

			RAssert(dtst_activate(v->server, ents[sid]));
			break;
		}
	}

	return true;
}
