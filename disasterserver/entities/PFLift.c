#include <entities/PFLift.h>

bool pflift_init(Server* server, Entity* entity)
{
	(void)server;
	
	PFLift* lift = (PFLift*)entity;
	lift->pos.y = lift->start;

	return true;
}

bool pflift_tick(Server* server, Entity* entity)
{
	Packet pack;
	PFLift* lift = (PFLift*)entity;

	if (!lift->activated)
	{
		if (lift->timer > 0)
		{
			lift->timer -= server->delta;
			if (lift->timer <= 0)
			{
				// teleport back
				PacketCreate(&pack, SERVER_PFLIFT_STATE);
				PacketWrite(&pack, packet_write8, 3);
				PacketWrite(&pack, packet_write8, lift->lid);
				PacketWrite(&pack, packet_write16, (uint16_t)lift->start);
				server_broadcast(server, &pack, true);
			}
		}
		return true;
	}

	if (lift->pos.y > lift->end)
	{
		if (lift->speed < 7.f)
			lift->speed += 0.052f * (float)server->delta;

		lift->pos.y -= lift->speed * (float)server->delta;
	}
	else
	{
		PacketCreate(&pack, SERVER_PFLIFT_STATE);
		PacketWrite(&pack, packet_write8, 2);
		PacketWrite(&pack, packet_write8, lift->lid);
		PacketWrite(&pack, packet_write16, lift->activator);
		PacketWrite(&pack, packet_write16, (uint16_t)lift->pos.y);
		server_broadcast(server, &pack, true);
		
		lift->timer = (uint16_t)(1.5f * TICKSPERSEC);
		lift->activated = false;
		lift->activator = 0;
	}

	PacketCreate(&pack, SERVER_PFLIFT_STATE);
	PacketWrite(&pack, packet_write8, 1);
	PacketWrite(&pack, packet_write8, lift->lid);
	PacketWrite(&pack, packet_write16, lift->activator);
	PacketWrite(&pack, packet_write16, (uint16_t)lift->pos.y);
	server_broadcast(server, &pack, false);

	return true;
}

bool pflift_activate(Server* server, PFLift* lift, uint16_t id)
{
	if (lift->activated)
		return true;

	if (lift->timer > 0)
		return true;

	lift->activator = id;
	lift->timer = 0;
	lift->speed = 0;
	lift->pos.y = lift->start;
	lift->activated = true;

	Packet pack;
	PacketCreate(&pack, SERVER_PFLIFT_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write8, lift->lid);
	PacketWrite(&pack, packet_write16, lift->activator);
	server_broadcast(server, &pack, true);

	return true;
}
