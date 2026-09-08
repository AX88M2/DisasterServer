#include <entities/Ring.h>
#include <stdint.h>
#include <CMath.h>

bool ring_init(Server* server, Entity* entity)
{
	Ring* ring = (Ring*)entity;

	int cnt = 0;
	for (int i = 0; i < g_mapList[server->game.map].ring_count; i++)
	{
		if (server->game.rings[i])
			cnt++;
	}

	if (cnt >= g_mapList[server->game.map].ring_count)
		return false;

	int rnd;
gen:
	rnd = rand() % g_mapList[server->game.map].ring_count;

	if (server->game.rings[rnd])
		goto gen;

	server->game.rings[rnd] = true;
	ring->rid = (uint8_t)rnd;
	ring->red = g_mapList[server->game.map].spawn_red_rings && (rand() % 100 <= 10);

	Packet pack;
	PacketCreate(&pack, SERVER_RING_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write8, ring->rid);
	PacketWrite(&pack, packet_write16, ring->id);
	PacketWrite(&pack, packet_write8, ring->red);
	server_broadcast(server, &pack, true);

	return true;
}

bool ring_uninit(Server* server, Entity* entity)
{
	Ring* ring = (Ring*)entity;
	server->game.rings[ring->rid] = 0;

	Packet pack;
	PacketCreate(&pack, SERVER_RING_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write8, ring->rid);
	PacketWrite(&pack, packet_write16, ring->id);
	server_broadcast(server, &pack, true);

	return true;
}