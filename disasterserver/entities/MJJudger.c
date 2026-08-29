#include <entities/MJJudger.h>

bool mjew_tick(Server* server, Entity* entity)
{
	MJew* ass = (MJew*)entity;
	if (ass->timer >= ass->next_time)
	{
		switch (ass->state)
		{
			case MJJ_WAIT:
			{
				ass->state = MJJ_PREPARE;
				ass->next_time = 2 * TICKSPERSEC;
				break;
			}

			case MJJ_PREPARE:
			{
				ass->state = MJJ_FIRE;
				ass->next_time = (7 + rand() % 2) * TICKSPERSEC;
				break;
			}

			case MJJ_FIRE:
			{
				ass->state = MJJ_WAIT;
				ass->next_time = (10 + rand() % 5) * TICKSPERSEC;
				break;
			}
		}

		Packet pack;
		PacketCreate(&pack, SERVER_MJJUDGER_STATE);
		PacketWrite(&pack, packet_write8, ass->state);
		server_broadcast(server, &pack, true);

		ass->timer = 0;
	}

	ass->timer += server->delta;
	return true;
}