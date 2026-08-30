#include <entities/NAPIce.h>

bool ice_tick(Server* server, Entity* entity)
{
	Ice* ice = (Ice*)entity;
	if (!ice->activated)
		return true;

	ice->timer -= server->delta;
	if (ice->timer <= 0)
	{
		ice->timer = 15.0 * TICKSPERSEC;
		ice->activated = false;

		Packet pack;
		PacketCreate(&pack, SERVER_NAPICE_STATE);
		PacketWrite(&pack, packet_write8, 1);
		PacketWrite(&pack, packet_write8, ice->iid);
		server_broadcast(server, &pack, true);
	}

	return true;
}

bool ice_activate(Server* server, Ice* ice)
{
	if (ice->activated)
		return true;

	ice->timer = 15.0 * TICKSPERSEC;
	ice->activated = true;

	Packet pack;
	PacketCreate(&pack, SERVER_NAPICE_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write8, ice->iid);
	server_broadcast(server, &pack, true);

	return true;
}
