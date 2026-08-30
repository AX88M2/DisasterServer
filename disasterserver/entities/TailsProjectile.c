#include <entities/TailsProjectile.h>

bool tproj_init(Server* server, Entity* entity)
{	
	TProjectile* tproj = (TProjectile*)entity;

	Packet pack;
	PacketCreate(&pack, SERVER_TPROJECTILE_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write16, (uint16_t)tproj->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)tproj->pos.y);
	PacketWrite(&pack, packet_write16, tproj->owner);
	PacketWrite(&pack, packet_write8, tproj->dir);
	PacketWrite(&pack, packet_write8, tproj->damage);
	PacketWrite(&pack, packet_write8, tproj->exe);
	PacketWrite(&pack, packet_write8, tproj->charge);
	server_broadcast(server, &pack, true);

	return true;
}

bool tproj_tick(Server* server, Entity* entity)
{
	TProjectile* tproj = (TProjectile*)entity;

	if (tproj->timer <= 0)
		return false;

	if (server->game.map != 13)
	{
		if (tproj->pos.x <= 0)
			return false;
	}
	else
	{
		if (tproj->pos.x < 0)
			tproj->pos.x = 3648;
		else if (tproj->pos.x > 3648)
			tproj->pos.x = 0;
	}

	Packet pack;
	PacketCreate(&pack, SERVER_TPROJECTILE_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write16, (uint16_t)tproj->pos.x);
	PacketWrite(&pack, packet_write16, (uint16_t)tproj->pos.y);
	server_broadcast(server, &pack, false);

	tproj->pos.x += (float)(tproj->dir * 14 * server->delta);
	tproj->timer -= server->delta;

	return true;
}

bool tproj_uninit(Server* server, Entity* entity)
{
	(void)entity;
	
	Packet pack;
	PacketCreate(&pack, SERVER_TPROJECTILE_STATE);
	PacketWrite(&pack, packet_write8, 2);
	server_broadcast(server, &pack, true);

	return true;
}