#include "HideAndSeek.hpp"

#include "Server.hpp"

using namespace DisasterServer::Maps;

HideAndSeek::HideAndSeek(MapContext context) : Map(context, "Hide And Seek") {}

void HideAndSeek::init()
{
}

void HideAndSeek::tick() {

}

void HideAndSeek::handle(Client&, Packet&) {

}

void HideAndSeek::left(Client&) {

}

DisasterServer::MapProperties HideAndSeek::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .build();
}