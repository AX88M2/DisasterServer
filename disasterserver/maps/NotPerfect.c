#include <maps/NotPerfect.h>
#include <entities/NotPerfect.h>
#include <stdlib.h>
#include <time.h>

bool np_init(Server* server)
{
    // Spawn controller
    RAssert(game_spawn(server, (Entity*)&(MakeNPCtrl()), sizeof(NPController), NULL));

    if (g_config.random_mode)
    {
        srand((unsigned int)time(NULL));
        int addTimeRandom = (rand() % 128) + 1; // 1-128
        RAssert(map_time(server, (2.585 + (addTimeRandom / 60.0)) * TICKSPERSEC, 20)); // 155 + addTimeRandom
    }
    else
    {
        RAssert(map_time(server, 2.585 * TICKSPERSEC, 20)); // 155
    }
    
    RAssert(map_ring(server, 3));
    
    return true;
}