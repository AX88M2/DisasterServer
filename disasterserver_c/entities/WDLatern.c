#include <entities/WDLatern.h>

bool latern_tick(Server* server, Entity* entity)
{
	Latern* lat = (Latern*)entity;

	lat->timer += server->delta;
	if (!lat->side)
	{
		if (lat->timer >= lat->time * TICKSPERSEC)
		{
			lat->lid = rand() % 7;

			Packet pack;
			PacketCreate(&pack, SERVER_WDLATERN_ACTIVATE);
			PacketWrite(&pack, packet_write8, true);
			PacketWrite(&pack, packet_write8, lat->lid);
			server_broadcast(server, &pack, true);

			lat->side = true;
			lat->timer = 0;
			lat->time = (20 + rand() % 2);
		}
	}
	else
	{
		if (lat->timer >= lat->time * TICKSPERSEC)
		{
			Packet pack;
			PacketCreate(&pack, SERVER_WDLATERN_ACTIVATE);
			PacketWrite(&pack, packet_write8, false);
			PacketWrite(&pack, packet_write8, lat->lid);
			server_broadcast(server, &pack, true);

			lat->side = false;
			lat->timer = 0;
			lat->time = (7 + rand() % 2);
		}
	}

	return true;
}