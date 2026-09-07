#include <entities/YouCantRun.h>
#include <CMath.h>

bool ycrctrl_tick(Server* server, Entity* entity)
{
	YCRController* ctrl = (YCRController*)entity;

	switch (ctrl->state)
	{
		case YCC_NONE:
		{
			if (ctrl->timer >= TICKSPERSEC)
			{
				Packet pack;
				PacketCreate(&pack, SERVER_YCRSMOKE_READY);
				PacketWrite(&pack, packet_write8, ctrl->smoke_id);
				server_broadcast(server, &pack, true);

				ctrl->state = YCC_SOME;
			}
			break;
		}

		case YCC_SOME:
		{
			if (ctrl->timer >= 6 * TICKSPERSEC)
			{
				ctrl->state = YCC_NONE;
				ctrl->timer = 0;
				ctrl->activated = !ctrl->activated;

				if (ctrl->activated)
					ctrl->smoke_id = rand() % 7;
				else
					ctrl->smoke_id = 0;

				Packet pack;
				PacketCreate(&pack, SERVER_YCRSMOKE_STATE);
				PacketWrite(&pack, packet_write8, ctrl->activated);
				PacketWrite(&pack, packet_write8, ctrl->smoke_id);
				server_broadcast(server, &pack, true);
			}
			break;
		}
	}

	ctrl->timer += server->delta;
	return true;
}