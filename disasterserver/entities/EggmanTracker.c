#include <entities/EggmanTracker.h>

bool eggtrack_init(Server* server, Entity* entity)
{
	EggTracker* egg = (EggTracker*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_ETRACKER_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write16, egg->id);
	PacketWrite(&pack, packet_write16, (uint16_t)egg->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)egg->pos.y);
	server_broadcast(server, &pack, true);
	return true;
}

bool eggtrack_uninit(Server* server, Entity* entity)
{
	EggTracker* egg = (EggTracker*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_ETRACKER_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write16, egg->id);
	PacketWrite(&pack, packet_write16, egg->activ_id);
	server_broadcast(server, &pack, true);
	return true;
}
