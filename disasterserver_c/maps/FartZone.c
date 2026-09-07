#include <maps/FartZone.h>
#include <entities/SpkieController.h>
#include <entities/DTBall.h>
#include <entities/BlackRing.h>
#include <entities/Dummy.h>
#include <States.h>

bool ft_init(Server* server)
{
	RAssert(map_time(server, 4.27 * TICKSPERSEC, 20));
	RAssert(map_ring(server, 1));

	for (uint8_t i = 0; i < 3; i++)
		RAssert(game_spawn(server, (Entity*)&(MakeBlackRing(MAP_BRING, MAP_BRING)), sizeof(BRing), NULL));

	RAssert(game_spawn(server, (Entity*)&(MakeDummy()), sizeof(Dummy), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeSpike()), sizeof(SpikeController), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeDTBall()), sizeof(DTBall), NULL));

	return true;
}

bool ft_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_FART_PUSH:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(spd, packet, packet_read8, int8_t);

			Dummy* dum;
			if (!game_find(v->server, (Entity**)&dum, "dummy", 1))
				break;

			dummy_activate(dum, spd);
			break;
		}
	}

	return true;
}
