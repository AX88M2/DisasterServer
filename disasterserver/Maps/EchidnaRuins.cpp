#include "EchidnaRuins.hpp"

#include "Entities/EchidnaRuins/MJCrystal.hpp"
#include "Entities/EchidnaRuins/MJew.hpp"
#include "Entities/EchidnaRuins/MLava.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

EchidnaRuins::EchidnaRuins(MapContext context) : Map(context, "Echidna Ruins") {
}

void EchidnaRuins::init() {
    state->getEntityController().spawnEntity<Entities::MLava>(Vector2(), 1624, 512);
    state->getEntityController().spawnEntity<Entities::MJCrystal>();
    state->getEntityController().spawnEntity<Entities::MJew>();
}

void EchidnaRuins::tick() {
}

void EchidnaRuins::handle(Client &client, Packet &packet) {
}

void EchidnaRuins::left(Client &client) {
}

MapProperties EchidnaRuins::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(29)
        .timer(3.42 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}
