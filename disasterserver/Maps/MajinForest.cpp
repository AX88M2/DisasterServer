#include "MajinForest.hpp"

#include "Server.hpp"

using namespace DisasterServer::Maps;

MajinForest::MajinForest(MapContext context): Map(context, "Majin Forest") {}

void MajinForest::init()
{
}

void MajinForest::tick()
{
}

void MajinForest::handle(Client&, Packet&)
{
}

void MajinForest::left(Client&)
{
}

DisasterServer::MapProperties MajinForest::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .timer(2.585 * TICKS_PER_SEC)
        .multiplier(10)
        .ringCoff(3)
        .build();
}
