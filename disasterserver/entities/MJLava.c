#include <entities/MJLava.h>
#include <CMath.h>

bool mlava_tick(Server* server, Entity* entity)
{
	MLava* lv = (MLava*)entity;
	switch (lv->state)
	{
	case MLV_IDLE: // Move using sin
	{
		lv->pos.y = lv->start + sinf((float)lv->timer / 25.0f) * 6;

		lv->timer -= server->delta;
		if (lv->timer <= 0)
			lv->state = MLV_MOVEDOWN;

		break;
	}

	case MLV_MOVEDOWN: // Move down a bit
	{
		if (lv->pos.y < lv->start + 20)
		{
			lv->pos.y += 0.15f * (float)server->delta;
		}
		else
			lv->state = MLV_RAISE;

		break;
	}

	case MLV_RAISE: // accelerate
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
			lv->state = MLV_MOVE;
			lv->timer = (4.0 + rand() % 3) * TICKSPERSEC;
			lv->vel = 0;
		}

		break;
	}

	case MLV_MOVE: // move on spot
	{
		lv->pos.y = (lv->start - lv->dist) + sinf((float)lv->timer / 25.0f) * 6;

		lv->timer -= server->delta;
		if (lv->timer <= 0)
			lv->state = MLV_LOWER;
		break;
	}

	case MLV_LOWER: // go down
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
			lv->state = MLV_IDLE;
			lv->timer = (5.0 + rand() % 4) * TICKSPERSEC;
			lv->vel = 0;
		}

		break;
	}
	}

	Packet pack;
	PacketCreate(&pack, SERVER_MJLAVA_STATE);
	PacketWrite(&pack, packet_write8, lv->state);
	PacketWrite(&pack, packet_writefloat, lv->pos.y);
	server_broadcast(server, &pack, false);

	return true;
}
