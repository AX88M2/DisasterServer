#include <maps/VolcanoValley.h>
#include <entities/VVVase.h>
#include <entities/VVLava.h>
#include <States.h>

bool vv_init(Server* server)
{
	RAssert(map_time(server, 3 * TICKSPERSEC, 20)); //180
	RAssert(map_ring(server, 5));

	for (uint8_t i = 0; i < 14; i++)
		RAssert(game_spawn(server, (Entity*)&(MakeVase(i)), sizeof(Vase), NULL));

	RAssert(game_spawn(server, (Entity*)&(MakeLava(0, 736, 130)), sizeof(Lava), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeLava(1, 1388, 130)), sizeof(Lava), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeLava(2, 1524, 130)), sizeof(Lava), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeLava(3, 1084, 130)), sizeof(Lava), NULL));

	return true;
}

bool vv_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_VVVASE_BREAK:
		{
			PacketRead(vid, packet, packet_read8, uint8_t);

			Vase* ents[14];
			int found = game_find(v->server, (Entity**)ents, "vase", 14);
			for (int i = 0; i < found; i++)
			{
				if (ents[i]->vid == vid)
				{
					Packet pack;
					PacketCreate(&pack, SERVER_VVVASE_STATE);
					PacketWrite(&pack, packet_write8, ents[i]->vid);
					PacketWrite(&pack, packet_write8, ents[i]->type);
					PacketWrite(&pack, packet_write16, v->id);
					server_broadcast(v->server, &pack);

					RAssert(game_despawn(v->server, NULL, ents[i]->id));
					break;
				}
			}
			break;
		}
	}

	return true;
}
