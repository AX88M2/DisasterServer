#include <maps/HideAndSeek2.h>
#include <States.h>

bool hs2_init(Server* server)
{
	RAssert(map_time(server, 3.42 * TICKSPERSEC, 20)); //205
	RAssert(map_ring(server, 5));

	return true;
}