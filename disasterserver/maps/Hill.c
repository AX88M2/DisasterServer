#include <maps/Hill.h>
#include <entities/HillThunder.h>
#include <States.h>
#include <CMath.h>

bool hill_init(Server* server)
{
	RAssert(map_time(server, 3 * TICKSPERSEC, 20));
	RAssert(map_ring(server, 5));

	RAssert(game_spawn(server, (Entity*)&(MakeThunder()), sizeof(Thunder), NULL));

	return true;
}