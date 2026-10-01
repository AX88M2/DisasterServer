#include <entities/RMZShard.h>

bool shard_init(Server* server, Entity* entity)
{
	Shard* shard = (Shard*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_RMZSHARD_STATE);
	PacketWrite(&pack, packet_write8, shard->spawned);
	PacketWrite(&pack, packet_write16, shard->id);
	PacketWrite(&pack, packet_write16, (uint16_t)shard->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)shard->pos.y);
	server_broadcast(server, &pack, true);

	return true;
}