#include <maps/HauntingDream.h>
#include <entities/HDDoor.h>
#include <States.h>

bool hd_init(Server* server)
{
	RAssert(map_time(server, 3.42 * TICKSPERSEC, 20)); //205
	RAssert(map_ring(server, 5));
	RAssert(game_spawn(server, (Entity*)&(MakeHDDoor()), sizeof(HDDoor), NULL));

	return true;
}

bool hd_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_HDDOOR_TOGGLE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			HDDoor* door;
			if (!game_find(v->server, (Entity**)&door, "hddoor", 1))
				break;
			
			hddoor_toggle(v->server, door);
			break;
		}
	}

	return true;
}
