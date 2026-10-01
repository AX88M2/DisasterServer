#include <maps/Marijuna.h>
#include <entities/MJLava.h>
#include <entities/MJAss.h>
#include <entities/MJJudger.h>

bool mj_init(Server* server)
{
	RAssert(map_time(server, 3.42 * TICKSPERSEC, 20)); //205
	RAssert(map_ring(server, 5));

	RAssert(game_spawn(server, (Entity*)&MakeMLava(1624, 512), sizeof(MLava), NULL));
	RAssert(game_spawn(server, (Entity*)&MakeMAss(), sizeof(MAss), NULL));
	RAssert(game_spawn(server, (Entity*)&MakeMJew(), sizeof(MJew), NULL));

	return true;
}