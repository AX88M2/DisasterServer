#include <maps/WoodDream.h>
#include <entities/WDLatern.h>

bool wd_init(Server* server)
{
	RAssert(map_time(server, 2.585 * TICKSPERSEC, 20)); //155
	RAssert(map_ring(server, 5));
	RAssert(game_spawn(server, (Entity*)&(MakeLatern()), sizeof(Latern), NULL));
	return true;
}
