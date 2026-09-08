#include <entities/HDDoor.h>

bool hddoor_tick(Server* server, Entity* entity)
{
	HDDoor* door = (HDDoor*)entity;
	if (door->timer > 0)
	{
		door->timer -= server->delta;
		if (door->timer <= 0)
		{
			Packet pack;
			PacketCreate(&pack, SERVER_HDDOOR_STATE);
			PacketWrite(&pack, packet_write8, 1);
			PacketWrite(&pack, packet_write8, 1);
			server_broadcast(server, &pack, true);
		}
	}

	return true;
}

bool hddoor_toggle(Server* server, HDDoor* door)
{
	if (door->timer > 0)
		return false;

	door->state = !door->state;
	door->timer = 10.0 * TICKSPERSEC;

	Packet pack;
	PacketCreate(&pack, SERVER_HDDOOR_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write8, door->state);
	server_broadcast(server, &pack, true);

	PacketCreate(&pack, SERVER_HDDOOR_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write8, 0);
	server_broadcast(server, &pack, true);

	return true;
}