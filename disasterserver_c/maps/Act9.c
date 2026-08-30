#include <maps/Act9.h>
#include <entities/Act9Wall.h>
#include <States.h>

bool act9_init(Server* server)
{
	RAssert(map_time(server, 2.17 * TICKSPERSEC, 10)); // 130
	RAssert(map_ring(server, 3));

	RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(0, 0, 1025)), sizeof(Act9Wall), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(1, 1663, 0)), sizeof(Act9Wall), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(2, 1663, 0)), sizeof(Act9Wall), NULL));
	return true;
}