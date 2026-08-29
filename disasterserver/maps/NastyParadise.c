#include <maps/NastyParadise.h>
#include <entities/NAPSnowball.h>
#include <entities/NAPIce.h>

bool nap_init(Server* server)
{
	Snowball* sb;

	RAssert(map_time(server, 2.585 * TICKSPERSEC, 20)); //155
	RAssert(map_ring(server, 5));

	for (uint8_t i = 0; i < 10; i++)
	{
		RAssert(game_spawn(server, (Entity*)&(MakeIce(i)), sizeof(Ice), NULL));
	}

	RAssert(game_spawn(server, (Entity*)&(MakeSnowball(0, 10, 1)), sizeof(Snowball), (Entity**)&sb));
	for (int i = 0; i < 4; i++)
	{
		sb->p_move[5 + i] = 0.05f + 0.05f * (i / 4.0f);
		sb->p_anim[5 + i] = 0.35f + 0.25f * (i / 4.0f);
	}

	RAssert(game_spawn(server, (Entity*)&(MakeSnowball(1, 8, -1)), sizeof(Snowball), (Entity**)&sb));
	for (int i = 0; i < 5; i++)
	{
		sb->p_move[2 + i] = 0.05f + 0.05f * (i / 5.0f);
		sb->p_anim[2 + i] = 0.35f + 0.25f * (i / 5.0f);
	}

	RAssert(game_spawn(server, (Entity*)&(MakeSnowball(2, 11, 1)), sizeof(Snowball), (Entity**)&sb));
	for (int i = 0; i < 5; i++)
	{
		sb->p_move[5 + i] = 0.05f + 0.05f * (i / 5.0f);
		sb->p_anim[5 + i] = 0.35f + 0.25f * (i / 5.0f);
	}

	RAssert(game_spawn(server, (Entity*)&(MakeSnowball(3, 9, 1)), sizeof(Snowball), (Entity**)&sb));
	for (int i = 0; i < 2; i++)
	{
		sb->p_move[6 + i] = 0.05f + 0.05f * (i / 2.0f);
		sb->p_anim[6 + i] = 0.35f + 0.25f * (i / 2.0f);
	}

	RAssert(game_spawn(server, (Entity*)&(MakeSnowball(4, 5, -1)), sizeof(Snowball), (Entity**)&sb));
	return true;
}

bool nap_tcpmsg(PeerData* v, Packet* packet)
{
	PacketRead(passtrough, packet, packet_read8, uint8_t);
	PacketRead(type, packet, packet_read8, uint8_t);

	switch (type)
	{
		case CLIENT_NAPICE_ACTIVATE:
		{
			AssertOrDisconnect(v->server, v->in_game);
			PacketRead(iid, packet, packet_read8, uint8_t);
			AssertOrDisconnect(v->server, iid < 10);

			Ice* ices[10];
			if (!game_find(v->server, (Entity**)ices, "ice", 10))
				break;
			
			RAssert(ice_activate(v->server, ices[iid]));
			break;
		}
	}

	return true;
}
