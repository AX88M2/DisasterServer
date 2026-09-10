#include "Maps.hpp"
#include "Server.hpp"

namespace DisasterServer
{
bool map_time(Server& server, double seconds, float mul)
{
    server.game.timeSec = static_cast<uint16_t>(seconds + ((server.getInGameCount() - 1) * mul));
    return true;
}
bool map_ring(Server& server, int ringcoff)
{
    auto& g = server.game;
    g.ringCoff = ringcoff;
    if (server.getInGameCount() > 3)
        g.ringCoff--;

    if (g.ringCoff < 1)
        g.ringCoff = 1;
    return true;
}
}