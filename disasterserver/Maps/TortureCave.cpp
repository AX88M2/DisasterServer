#include "TortureCave.hpp"

#include "States/GameState.hpp"
#include "Entities/TortureCave/Acid.hpp"

using namespace DisasterServer::Maps;

TortureCave::TortureCave(MapContext context) : Map(context, "Torture Cave") {
}

void TortureCave::init() {
    state->getEntityController().spawnEntity<Entities::Acid>();
}

void TortureCave::tick() {
}

void TortureCave::handle(Client &client, Packet &packet) {
}

void TortureCave::left(Client &client) {
}

DisasterServer::MapProperties TortureCave::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(27)
        .timer(2.585 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}
