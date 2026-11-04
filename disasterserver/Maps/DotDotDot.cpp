#include "DotDotDot.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/YouCantRun/SpikeController.hpp"

using namespace DisasterServer::Maps;

DotDotDot::DotDotDot(MapContext context) : Map(context, "...") {}

void DotDotDot::init() {
    state->getEntityController().spawnEntity<Entities::SpikeController>();
}
void DotDotDot::tick() {

}
void DotDotDot::handle(Client&, Packet&) {

}
void DotDotDot::left(Client&) {

}

DisasterServer::MapProperties DotDotDot::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .timer(3.42 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}