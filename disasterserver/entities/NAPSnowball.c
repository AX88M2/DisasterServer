#include <entities/NAPSnowball.h>

#define NUM_FRAMES 32
#define ROLL_START 16

bool snowball_init(Server* server, Entity* entity)
{
	(void)server;
	
	Snowball* sb = (Snowball*)entity;

	for (int i = 0; i < 20; i++)
	{
		sb->p_move[i] = 0.05f;
		sb->p_anim[i] = 0.35f;
	}

	return true;
}

bool snowball_tick(Server* server, Entity* entity)
{
	Snowball* sb = (Snowball*)entity;
	
	if (sb->timer < 20 * TICKSPERSEC)
		sb->timer += server->delta;
	else
	{
		RAssert(snowball_activate(server, sb));
		sb->timer = 0;
	}

	if (sb->active)
	{
		if (sb->vel > 1)
		{
			sb->frame += sb->p_anim[sb->state] * server->delta;
			sb->stage_prog += sb->p_move[sb->state] * server->delta;

			if (sb->frame >= NUM_FRAMES)
				sb->frame = ROLL_START;
		}
		else
		{
			sb->vel += 0.016 * server->delta;
			sb->frame += sb->vel * 0.45 * server->delta;
			sb->stage_prog += sb->vel * 0.05 * server->delta;
		}

		if (sb->stage_prog > 1)
		{
			sb->stage_prog = 0;
			sb->state++;

			if (sb->state >= sb->p_count - 1)
			{
				sb->state = 0;
				sb->stage_prog = 0;
				sb->active = false;
				sb->frame = 0;

				Packet pack;
				PacketCreate(&pack, SERVER_NAPBALL_STATE);
				PacketWrite(&pack, packet_write8, 2);
				PacketWrite(&pack, packet_write8, sb->sid);
				server_broadcast(server, &pack, true);

				return true;
			}
		}

		Packet pack;
		PacketCreate(&pack, SERVER_NAPBALL_STATE);
		PacketWrite(&pack, packet_write8, 1);
		PacketWrite(&pack, packet_write8, sb->sid);
		PacketWrite(&pack, packet_write8, sb->state);
		PacketWrite(&pack, packet_write8, (uint8_t)sb->frame);
		PacketWrite(&pack, packet_writedouble, sb->stage_prog);
		server_broadcast(server, &pack, false);
	}

	return true;
}

bool snowball_activate(Server* server, Snowball* sb)
{
	if (sb->active)
		return true;

	sb->vel = 0;
	sb->state = 0;
	sb->stage_prog = 0;
	sb->frame = 0;
	sb->active = true;

	Packet pack;
	PacketCreate(&pack, SERVER_NAPBALL_STATE);
	PacketWrite(&pack, packet_write8, 0);
	PacketWrite(&pack, packet_write8, sb->sid);
	PacketWrite(&pack, packet_write8, sb->dir);
	server_broadcast(server, &pack, true);

	return true;
}
