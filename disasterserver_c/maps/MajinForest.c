#include <maps/MajinForest.h>
#include <States.h>

bool maj_init(Server* server)
{
	RAssert(map_time(server, 2.585 * TICKSPERSEC, 10)); //155
	RAssert(map_ring(server, 3));
	return true;
}
