#include <maps/KindAndFair.h>
#include <entities/KAFSpeedBox.h>
#include <States.h>

bool kaf_init(Server* server)
{
	RAssert(map_time(server, 3 * TICKSPERSEC, 20)); //180
	RAssert(map_ring(server, 5));

	for (uint8_t i = 0; i < 11; i++)
		RAssert(game_spawn(server, (Entity*)&(MakeKafBox(i)), sizeof(KafBox), NULL));

	return true;
}

bool kaf_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_KAFMONITOR_ACTIVATE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(nid, packet, packet_read8, uint8_t);
			PacketRead(proj, packet, packet_read8, uint8_t);

			KafBox* box[11];
			if (!game_find(v->server, (Entity**)box, "kafbox", 11))
				break;

			RAssert(kafbox_activate(v->server, box[nid], v->id, proj));
			break;
		}
	}

	return true;
}
