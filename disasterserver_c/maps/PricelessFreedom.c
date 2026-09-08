#include <maps/PricelessFreedom.h>
#include <entities/BlackRing.h>
#include <entities/PFLift.h>
#include <States.h>

bool pf_init(Server* server)
{
	RAssert(map_ring(server, 5));
	RAssert(map_time(server, 2.585 * TICKSPERSEC, 10)); //155

	for (int i = 0; i < 29; i++)
		RAssert(game_spawn(server, (Entity*)&(MakeBlackRing(MAP_BRING, MAP_BRING)), sizeof(BRing), NULL));

	RAssert(game_spawn(server, (Entity*)&(MakePFLift(0, 1669, 1016)), sizeof(PFLift), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakePFLift(1, 1069, 704)),  sizeof(PFLift), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakePFLift(2, 829, 400)),   sizeof(PFLift), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakePFLift(3, 1070, 544)),  sizeof(PFLift), NULL));
	return true;
}

bool pf_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_PFLIT_ACTIVATE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(lid, packet, packet_read8, uint8_t);

			PFLift* ents[4];
			int found = game_find(v->server, (Entity**)ents, "pflift", 4);
			for (int i = 0; i < found; i++)
			{
				if (ents[i]->lid == lid)
				{
					pflift_activate(v->server, ents[i], v->id);
					break;
				}
			}
			break;
		}
	}

	return true;
}
