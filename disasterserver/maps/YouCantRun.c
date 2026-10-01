#include <maps/YouCantRun.h>
#include <entities/YouCantRun.h>
#include <entities/SpkieController.h>

bool ycr_init(Server* server)
{
	RAssert(map_time(server, 3 * TICKSPERSEC, 20)); //180
	RAssert(map_ring(server, 5));

	RAssert(game_spawn(server, (Entity*)&(MakeSpike()), sizeof(SpikeController), NULL));
	RAssert(game_spawn(server, (Entity*)&(MakeYCRCtrl()), sizeof(YCRController), NULL));
	
	return true;
}