#include "NotPerfect.hpp"

#include "Entities/NotPerfect/NPController.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

NotPerfect::NotPerfect(MapContext context) : Map(context, "Not Perfect") {
}

NotPerfect::~NotPerfect() noexcept = default;

void NotPerfect::init() {
    state->getEntityController().spawnEntity<NPController>();
}

void NotPerfect::tick() {
}

void NotPerfect::handle(Client &client, Packet &packet) {
}

void NotPerfect::left(Client &client) {
}

MapProperties NotPerfect::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(59)
        .timer(2.585 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}