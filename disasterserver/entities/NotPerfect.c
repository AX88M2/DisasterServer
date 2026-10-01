#include <entities/NotPerfect.h>
#include <math.h>

bool npctrl_tick(Server* server, Entity* entity)
{
	NPController* ctrl = (NPController*)entity;
	if (server->game.time_sec <= TICKSPERSEC && !ctrl->balls)
	{
		ctrl->timer = 5 * TICKSPERSEC;
		ctrl->state = NPC_PREPARE;
		ctrl->balls = true;
	}

	switch (ctrl->state)
	{
		case NPC_NONE:
		{
			const int intr1 = server->game.time_sec < TICKSPERSEC ? 2 : 15;
			if (ctrl->timer >= intr1 * TICKSPERSEC)
			{
				Packet pack;

				PacketCreate(&pack, SERVER_NPCONTROLLER_STATE);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write8, 0);
				PacketWrite(&pack, packet_write8, 0);
				server_broadcast(server, &pack, true);

				ctrl->state = NPC_PREPARE;
				ctrl->timer = 0;
			}
			break;
		}

		case NPC_PREPARE:
		{
			const int intr2 = server->game.time_sec < TICKSPERSEC ? 3 : 5;
			if (ctrl->timer >= intr2 * TICKSPERSEC)
			{
				ctrl->stage++;

				Packet pack;
				PacketCreate(&pack, SERVER_NPCONTROLLER_STATE);
				PacketWrite(&pack, packet_write8, 1);
				PacketWrite(&pack, packet_write8, ctrl->stage % 4);
				PacketWrite(&pack, packet_write8, (uint8_t)fmax(ctrl->stage - 1, 0) % 4);
				server_broadcast(server, &pack, true);

				ctrl->state = NPC_NONE;
				ctrl->timer = 0;
			}
			break;
		}
	}

	ctrl->timer += server->delta;
	return true;
}