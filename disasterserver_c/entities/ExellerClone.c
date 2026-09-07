#include <entities/ExellerClone.h>

bool exclone_init(Server* server, Entity* entity)
{
	ExellerClone* clone = (ExellerClone*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_EXELLERCLONE_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write16, clone->id);
	PacketWrite(&pack, packet_write16, clone->owner);
	PacketWrite(&pack, packet_write16, (uint16_t)clone->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)clone->pos.y);
	PacketWrite(&pack, packet_write8,  clone->dir);
	server_broadcast(server, &pack, true);

	return true;
}

bool exclone_uninit(Server* server, Entity* entity)
{
	return true;
}
