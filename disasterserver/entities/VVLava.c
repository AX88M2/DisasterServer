#include <entities/VVLava.h>
#include <CMath.h>

bool lava_tick(Server* server, Entity* entity)
{
	Lava* lv = (Lava*)entity;
	switch (lv->state)
	{
		case LV_IDLE: // Move using sin
		{
			lv->pos.y = lv->start + sinf((float)lv->timer / 25.0f) * 6;

			lv->timer -= server->delta;
			if (lv->timer <= 0)
				lv->state = LV_MOVEDOWN;

			break;
		}

		case LV_MOVEDOWN: // Move down a bit
		{
			if (lv->pos.y < lv->start + 20)
			{
				lv->pos.y += 0.15f * (float)server->delta;
			}
			else
				lv->state = LV_RAISE;

			break;
		}

		case LV_RAISE: // accelerate
		{
			if (lv->pos.y > lv->start - lv->dist)
			{
				lv->pos.y -= lv->vel;

				if (lv->vel < 5)
					lv->vel += 0.08f * (float)server->delta;
				else
					lv->vel = 5;
			}
			else
			{
				lv->state = LV_MOVE;
				lv->timer = (4.0 + rand() % 3) * TICKSPERSEC;
				lv->vel = 0;
			}

			break;
		}

		case LV_MOVE: // move on spot
		{
			lv->pos.y = (lv->start - lv->dist) + sinf((float)lv->timer / 25.0f) * 6;

			lv->timer -= server->delta;
			if (lv->timer <= 0)
				lv->state = LV_LOWER;
			break;
		}

		case LV_LOWER: // go down
		{
			if (lv->start > lv->pos.y)
			{
				lv->pos.y += lv->vel * (float)server->delta;

				if (lv->vel < 5)
					lv->vel += 0.08f * (float)server->delta;
				else
					lv->vel = 5;
			}
			else
			{
				lv->state = LV_IDLE;
				lv->timer = (20.0 + rand() % 5) * TICKSPERSEC;
				lv->vel = 0;
			}

			break;
		}
	}

	Packet pack;
	PacketCreate(&pack, SERVER_VVLCOLUMN_STATE);
	PacketWrite(&pack, packet_write8, lv->lid);
	PacketWrite(&pack, packet_write8, lv->state);
	PacketWrite(&pack, packet_writefloat, lv->pos.y);
	server_broadcast(server, &pack, false);

	return true;
}
