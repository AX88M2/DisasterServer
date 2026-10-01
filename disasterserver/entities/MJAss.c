#include <entities/MJAss.h>

bool mass_tick(Server* server, Entity* entity)
{
	MAss* ass = (MAss*)entity;

	if (ass->timer >= ass->next_time)
	{
		ass->state = !ass->state;
		ass->timer = 0;

		Packet pack;
		PacketCreate(&pack, SERVER_MJCRYSTAL_STATE);
		PacketWrite(&pack, packet_write8, ass->state);
		server_broadcast(server, &pack, true);

		ass->next_time = (10 + rand() % 5) * TICKSPERSEC;
	}

	ass->timer += server->delta;
	return true;
}