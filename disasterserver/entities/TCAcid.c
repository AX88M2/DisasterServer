#include <entities/TCAcid.h>
#include <CMath.h>

bool acid_tick(Server* server, Entity* entity)
{
	Acid* ac = (Acid*)entity;

	if (ac->timer >= (4 * TICKSPERSEC))
	{
		ac->timer = 0;
		ac->activated = !ac->activated;

		if (ac->activated)
			ac->acid_id = rand() % 7;

		Packet pack;
		PacketCreate(&pack, SERVER_TCGOM_STATE);
		PacketWrite(&pack, packet_write8, ac->acid_id);
		PacketWrite(&pack, packet_write8, ac->activated);
		server_broadcast(server, &pack, true);
	}

	ac->timer += server->delta;
	return true;
}
