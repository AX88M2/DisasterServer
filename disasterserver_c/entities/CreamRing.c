#include <entities/CreamRing.h>
#include <stdint.h>
#include <CMath.h>

bool cring_init(Server* server, Entity* entity)
{
	CreamRing* ring = (CreamRing*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_RING_STATE);
	PacketWrite(&pack, packet_write8, 2);
	PacketWrite(&pack, packet_write16, (uint16_t)ring->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)ring->pos.y);
	PacketWrite(&pack, packet_write8, ring->rid);
	PacketWrite(&pack, packet_write16, ring->id);
	PacketWrite(&pack, packet_write8, ring->red);
	server_broadcast(server, &pack, true);

	return true;
}

bool cring_uninit(Server* server, Entity* entity)
{
	CreamRing* ring = (CreamRing*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_RING_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write8, ring->rid);
	PacketWrite(&pack, packet_write16, ring->id);
	server_broadcast(server, &pack, true);

	return true;
}