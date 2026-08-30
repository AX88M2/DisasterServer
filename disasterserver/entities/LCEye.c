#include <entities/LCEye.h>

bool lceye_tick(Server* server, Entity* entity)
{
	LCEye* eye = (LCEye*)entity;

	if (eye->cooldown > 0)
	{
		eye->cooldown -= server->delta;
		return true;
	}

	if (eye->timer >= TICKSPERSEC)
	{
		if (eye->used && eye->charge > 0)
		{
			eye->charge -= 20;
			if (eye->charge < 20)
			{
				eye->cooldown = 2.0 * TICKSPERSEC;
				eye->used = eye->timer = 0;
			}

			RAssert(lceye_update(server, eye));
		}
		else if (!eye->used && eye->charge < 100)
		{
			eye->charge += 10;
			RAssert(lceye_update(server, eye));
		}

		eye->timer = 0;
	}

	eye->timer += server->delta;
	return true;
}

bool lceye_update(Server* server, LCEye* eye)
{
	Packet pack;
	PacketCreate(&pack, SERVER_LCEYE_STATE);
	PacketWrite(&pack, packet_write8,  (uint8_t)eye->eye_id);
	PacketWrite(&pack, packet_write8,  eye->used);
	PacketWrite(&pack, packet_write16, eye->use_id);
	PacketWrite(&pack, packet_write8,  (uint8_t)eye->target);
	PacketWrite(&pack, packet_write8,  eye->charge);
	server_broadcast(server, &pack, true);

	return true;
}
