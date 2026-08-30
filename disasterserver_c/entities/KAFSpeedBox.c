#include <entities/KAFSpeedBox.h>

bool kafbox_init(Server* server, Entity* entity)
{
	KafBox* box = (KafBox*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_KAFMONITOR_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write8, box->nid);
	server_broadcast(server, &pack, true);

	return true;
}

bool kafbox_tick(Server* server, Entity* entity)
{
	KafBox* box = (KafBox*)entity;

	if (!box->activated)
		return true;

	if (box->timer > 0)
	{
		box->timer -= server->delta;
		return true;
	}

	Packet pack;
	PacketCreate(&pack, SERVER_KAFMONITOR_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write8, box->nid);
	server_broadcast(server, &pack, true);

	box->activated = false;
	return true;
}

bool kafbox_activate(Server* server, KafBox* box, uint16_t pid, uint8_t is_proj)
{
	if (box->activated)
		return true;

	Packet pack;
	PacketCreate(&pack, SERVER_KAFMONITOR_STATE);
	PacketWrite(&pack, packet_write8, 2);
	PacketWrite(&pack, packet_write8, box->nid);
	PacketWrite(&pack, packet_write16, is_proj ? 0 : pid);
	server_broadcast(server, &pack, true);

	box->activated = true;
	box->timer = (25.0 + rand() % 5) * TICKSPERSEC;

	return true;
}
