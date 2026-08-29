#include <maps/LimpCity.h>
#include <entities/LCEye.h>
#include <entities/LCChain.h>

bool lc_init(Server* server)
{
	RAssert(map_time(server, 2.585 * TICKSPERSEC, 20)); //155
	RAssert(map_ring(server, 5));

	RAssert(game_spawn(server, (Entity*)&(MakeLCEye(0)), sizeof(LCEye), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeLCEye(1)), sizeof(LCEye), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeLCChain()), sizeof(LCChain), NULL));

	return true;
}

bool lc_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_LCEYE_REQUEST_ACTIVATE:
		{
			PacketRead(val, packet, packet_read8, uint8_t);
			PacketRead(nid, packet, packet_read8, uint8_t);
			PacketRead(target, packet, packet_read8, uint8_t);
			AssertOrDisconnect(v->server, nid < 2);
			AssertOrDisconnect(v->server, v->in_game);

			LCEye* eyes[2];
			if (!game_find(v->server, (Entity**)eyes, "lceye", 2))
				break;

			LCEye* eye = eyes[nid];
			if (val)
			{
				if (eye->used)
					break;

				if (eye->charge < 20)
					break;

				eye->use_id = v->id;
				eye->target = target;
				eye->used = true;
				eye->timer = 0;

				RAssert(lceye_update(v->server, eye));
			}
			else
			{
				eye->used = false;
				eye->timer = 0;

				RAssert(lceye_update(v->server, eye));
			}

			break;
		}
	}

	return true;
}
