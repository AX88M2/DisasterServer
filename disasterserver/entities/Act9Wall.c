#include <entities/Act9Wall.h>

bool act9wall_init(Server* server, Entity* entity)
{
	Act9Wall* wall = (Act9Wall*)entity;
	wall->start_time = (server->game.time_sec * TICKSPERSEC + server->game.time);

	return true;
}

bool act9wall_tick(Server* server, Entity* entity)
{
	Act9Wall* wall = (Act9Wall*)entity;

	double time = (server->game.time_sec * TICKSPERSEC + server->game.time);
	double off = (double)(wall->start_time - time) / (double)wall->start_time;

	uint16_t x = (uint16_t)(wall->pos.x * off);
	uint16_t y = (uint16_t)(wall->pos.y * off);
	
	Packet pack;
	PacketCreate(&pack, SERVER_ACT9WALL_STATE);
	PacketWrite(&pack, packet_write8, wall->wid);
	PacketWrite(&pack, packet_write16, x);
	PacketWrite(&pack, packet_write16, y);
	game_broadcast(server, &pack);

	return true;
}