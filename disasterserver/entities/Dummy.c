#include <entities/Dummy.h>
#include <CMath.h>

bool dummy_tick(Server* server, Entity* entity)
{
	Dummy* dum = (Dummy*)entity;
	
	dum->pos.x += dum->vel * (float)server->delta;
	dum->pos.x = (float)fmin(fmax(dum->pos.x, 1282), 2944);
	dum->vel -= (fmin(fabs(dum->vel), 0.046875 * 4.) * sign(dum->vel)) * (float)server->delta;

	Packet pack;
	PacketCreate(&pack, SERVER_FART_STATE);
	PacketWrite(&pack, packet_write16, (float)dum->pos.x);
	PacketWrite(&pack, packet_write16, (float)dum->pos.y);
	server_broadcast(server, &pack, false);

	return true;
}

void dummy_activate(Dummy* dummy, int8_t dir)
{
	dummy->vel = dir;
}
