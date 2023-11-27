#include <maps/NotPerfect.h>
#include <entities/NotPerfect.h>

bool np_init(Server* server)
{
	RAssert(map_time(server, 2.585 * TICKSPERSEC, 20)); //155
	RAssert(map_ring(server, 3));

	RAssert(game_spawn(server, (Entity*)&(MakeNPCtrl()), sizeof(NPController), NULL));
	return true;
}