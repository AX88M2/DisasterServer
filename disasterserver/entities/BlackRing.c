#include <entities/BlackRing.h>

bool bring_init(Server* server, Entity* entity)
{
	if (entity->pos.x == MAP_BRING || entity->pos.y == MAP_BRING)
	{
		Packet pack;
		PacketCreate(&pack, SERVER_BRING_STATE)
		PacketWrite(&pack, packet_write8, 0);
		PacketWrite(&pack, packet_write16, entity->id);
		server_broadcast(server, &pack, true);

		return true;
	}

	Packet pack;
	PacketCreate(&pack, SERVER_ERECTOR_BRING_SPAWN);
	PacketWrite(&pack, packet_write16, entity->id);
	PacketWrite(&pack, packet_write16, (uint16_t)entity->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)entity->pos.y);
	server_broadcast(server, &pack, true);

	return true;
}

bool bring_uninit(Server* server, Entity* entity)
{
	Packet pack;
	PacketCreate(&pack, SERVER_BRING_STATE)
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write16, entity->id);
	server_broadcast(server, &pack, true);

	return true;
}
