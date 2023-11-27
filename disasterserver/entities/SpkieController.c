#include <entities/SpkieController.h>

bool spike_tick(Server* server, Entity* entity)
{
	SpikeController* ctrl = (SpikeController*)entity;

	if (--ctrl->timer > 0)
		return true;

	if (++ctrl->frame > 5)
		ctrl->frame = 0;
	
	if (ctrl->frame == 0 || ctrl->frame == 2)
		ctrl->timer = 2 * TICKSPERSEC;
	else
		ctrl->timer = 0;

	Packet pack;
	PacketCreate(&pack, SERVER_MOVINGSPIKE_STATE);
	PacketWrite(&pack, packet_write8, ctrl->frame);
	server_broadcast(server, &pack);

	return true;
}
