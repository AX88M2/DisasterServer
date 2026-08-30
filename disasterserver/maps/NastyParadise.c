#include <maps/NastyParadise.h>
#include <entities/NAPSnowball.h>
#include <entities/NAPIce.h>
#include <stdlib.h>
#include <time.h>

static int nap_timer = 0;

bool nap_init(Server* server)
{
    Snowball* sb;

    // Spawn ice
    for (uint8_t i = 0; i < 10; i++)
    {
        RAssert(game_spawn(server, (Entity*)&(MakeIce(i)), sizeof(Ice), NULL));
    }

    // Snowball 0
    RAssert(game_spawn(server, (Entity*)&(MakeSnowball(0, 10, 1)), sizeof(Snowball), (Entity**)&sb));
    for (int i = 0; i < 4; i++)
    {
        sb->p_move[5 + i] = 0.05f + 0.05f * (i / 4.0f);
        sb->p_anim[5 + i] = 0.35f + 0.25f * (i / 4.0f);
    }

    // Snowball 1
    RAssert(game_spawn(server, (Entity*)&(MakeSnowball(1, 8, -1)), sizeof(Snowball), (Entity**)&sb));
    for (int i = 0; i < 5; i++)
    {
        sb->p_move[2 + i] = 0.05f + 0.05f * (i / 5.0f);
        sb->p_anim[2 + i] = 0.35f + 0.25f * (i / 5.0f);
    }

    // Snowball 2
    RAssert(game_spawn(server, (Entity*)&(MakeSnowball(2, 11, 1)), sizeof(Snowball), (Entity**)&sb));
    for (int i = 0; i < 5; i++)
    {
        sb->p_move[5 + i] = 0.05f + 0.05f * (i / 5.0f);
        sb->p_anim[5 + i] = 0.35f + 0.25f * (i / 5.0f);
    }

    // Snowball 3
    RAssert(game_spawn(server, (Entity*)&(MakeSnowball(3, 9, 1)), sizeof(Snowball), (Entity**)&sb));
    for (int i = 0; i < 2; i++)
    {
        sb->p_move[6 + i] = 0.05f + 0.05f * (i / 2.0f);
        sb->p_anim[6 + i] = 0.35f + 0.25f * (i / 2.0f);
    }

    // Snowball 4
    RAssert(game_spawn(server, (Entity*)&(MakeSnowball(4, 5, -1)), sizeof(Snowball), (Entity**)&sb));

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
    
    RAssert(map_ring(server, 5));
    nap_timer = 0;
    
    return true;
}

void nap_tick(Server* server)
{
    nap_timer++;
    if (nap_timer >= 1200)
    {
        nap_timer = 0;
        Snowball* snowballs[5];
        int count = game_find(server, (Entity**)snowballs, "snowball", 5);
        for (int i = 0; i < count; i++)
        {
            snowball_activate(server, snowballs[i]);
        }
    }
}

bool nap_tcpmsg(PeerData* v, Packet* packet)
{
    PacketRead(passtrough, packet, packet_read8, uint8_t);
    PacketRead(type, packet, packet_read8, uint8_t);

    switch (type)
    {
        case CLIENT_NAPICE_ACTIVATE:
        {
            AssertOrDisconnect(v->server, v->in_game);
            PacketRead(iid, packet, packet_read8, uint8_t);
            AssertOrDisconnect(v->server, iid < 10);

            Ice* ices[10];
            if (!game_find(v->server, (Entity**)ices, "ice", 10))
                break;
            
            RAssert(ice_activate(v->server, ices[iid]));
            break;
        }
    }

    return true;
}