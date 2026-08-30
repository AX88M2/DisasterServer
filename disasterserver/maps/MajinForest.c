#include <maps/MajinForest.h>
#include <States.h>
#include <stdlib.h>
#include <time.h>

bool maj_init(Server* server)
{
    if (g_config.random_mode)
    {
        srand((unsigned int)time(NULL));
        int addTimeRandom = (rand() % 128) + 1; // 1-128
        RAssert(map_time(server, (2.585 + (addTimeRandom / 60.0)) * TICKSPERSEC, 10)); // 155 + addTimeRandom
    }
    else
    {
        RAssert(map_time(server, 2.585 * TICKSPERSEC, 10)); // 155
    }
    
    RAssert(map_ring(server, 3));
    
    return true;
}