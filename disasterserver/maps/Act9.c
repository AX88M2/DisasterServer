#include <maps/Act9.h>
#include <entities/Act9Wall.h>
#include <States.h>
#include <stdlib.h>
#include <time.h>

bool act9_init(Server* server)
{
    if (g_config.random_mode)
    {
        // init the random number generator
        srand((unsigned int)time(NULL));

        int wallRandom = (rand() % 8) + 1; // 1-8
        int addTimeRandom = (rand() % 128) + 1; // 1-128
        RAssert(map_time(server, (2.17 + (addTimeRandom / 60.0)) * TICKSPERSEC, 10));
        
        // Spawn walls according to random selection
        switch (wallRandom)
        {
            case 1: // Ceiling + left + Right
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(0, 0, 1025)), sizeof(Act9Wall), NULL));
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(1, 1663, 0)), sizeof(Act9Wall), NULL));
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(2, 1663, 0)), sizeof(Act9Wall), NULL));
                break;
            case 2: // Ceiling + left
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(0, 0, 1025)), sizeof(Act9Wall), NULL));
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(1, 1663, 0)), sizeof(Act9Wall), NULL));
                break;
            case 3: // Ceiling + Right
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(0, 0, 1025)), sizeof(Act9Wall), NULL));
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(2, 1663, 0)), sizeof(Act9Wall), NULL));
                break;
            case 4: // Left + Right
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(1, 1663, 0)), sizeof(Act9Wall), NULL));
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(2, 1663, 0)), sizeof(Act9Wall), NULL));
                break;
            case 5: // only ceiling
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(0, 0, 1025)), sizeof(Act9Wall), NULL));
                break;
            case 6: // only left
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(1, 1663, 0)), sizeof(Act9Wall), NULL));
                break;
            case 7: // only right
                RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(2, 1663, 0)), sizeof(Act9Wall), NULL));
                break;
            case 8: // nothing (no walls)
                break;
        }
    }
    else
    {
        RAssert(map_time(server, 2.17 * TICKSPERSEC, 10));
        RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(0, 0, 1025)), sizeof(Act9Wall), NULL));
        RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(1, 1663, 0)), sizeof(Act9Wall), NULL));
        RAssert(game_spawn(server, (Entity*)&(MakeAct9Wall(2, 1663, 0)), sizeof(Act9Wall), NULL));
    }
    
    RAssert(map_ring(server, 3));
    
    return true;
}