#include "Hill.hpp"

#include "Entities/Hill/Thunder.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

Hill::Hill(MapContext context) : Map(context, "Hill") {
}

void Hill::init() {
    state->getEntityController().spawnEntity<Entities::Thunder>();
}

void Hill::tick() {
}

void Hill::handle(Client &client, Packet &packet) {
}

void Hill::left(Client &client) {
}

MapProperties Hill::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(26)
        .build();
}
