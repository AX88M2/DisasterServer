#include <maps/DotDotDot.h>
#include <entities/SpkieController.h>
#include <States.h>

bool dot_init(Server* server)
{
	RAssert(map_time(server, 3.42 * TICKSPERSEC, 20)); //205
	RAssert(map_ring(server, 5));
	RAssert(game_spawn(server, (Entity*)&(MakeSpike()), sizeof(SpikeController), NULL));

	return true;
}
