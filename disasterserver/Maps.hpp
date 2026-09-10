#pragma once
#include "Server.hpp"

namespace DisasterServer
{
    bool map_time(Server& server, double seconds, float mul);
    bool map_ring(Server& server, int ringcoff);
}