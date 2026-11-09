#include "MysticWood.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Entities/MysticWood/WDLatern.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

MysticWood::MysticWood(MapContext context) : Map(context, "Mystic Wood") {}

void MysticWood::init() {
    state->getEntityController().spawnEntity<Entities::Latern>();
}

void MysticWood::tick() {}
void MysticWood::handle(Client&, Packet&) {}
void MysticWood::left(Client&) {}

MapProperties MysticWood::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .timer(2.585 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}