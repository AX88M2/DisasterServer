#include "DesertTown.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

DesertTown::DesertTown(MapContext context) : Map(context, "Desert Town") {}

void DesertTown::init()
{
}

void DesertTown::tick() {

}

void DesertTown::handle(Client&, Packet&) {

}

void DesertTown::left(Client&) {

}

MapProperties DesertTown::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .build();
}
