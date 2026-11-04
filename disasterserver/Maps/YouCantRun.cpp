#include "YouCantRun.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/YouCantRun/SpikeController.hpp"
#include "Entities/YouCantRun/YCRController.hpp"

using namespace DisasterServer::Maps;

YouCantRun::YouCantRun(MapContext context) : Map(context, "You Can't Run") {}

void YouCantRun::init() {
    state->getEntityController().spawnEntity<Entities::SpikeController>();
    state->getEntityController().spawnEntity<Entities::YCRController>();
    Debug("Spawned SpikeController and YCRController");
}

void YouCantRun::tick() {

}
void YouCantRun::handle(Client&, Packet&) {

}
void YouCantRun::left(Client&) {

}

DisasterServer::MapProperties YouCantRun::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .timer(3 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}