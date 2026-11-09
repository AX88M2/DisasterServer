#include "HideAndSeekAct2.hpp"

#include "Server.hpp"

using namespace DisasterServer::Maps;

HideAndSeekAct2::HideAndSeekAct2(MapContext context) : Map(context, "Hide And Seek 2") {}

void HideAndSeekAct2::init()
{
}

void HideAndSeekAct2::tick() {

}

void HideAndSeekAct2::handle(Client&, Packet&) {

}

void HideAndSeekAct2::left(Client&) {

}

DisasterServer::MapProperties HideAndSeekAct2::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .timer(3.42 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}
