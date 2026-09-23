#include "DotDotDot.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/Spike.hpp"

using namespace DisasterServer::Maps;

DotDotDot::DotDotDot() : Map("...", 1, 30) {}

void DotDotDot::init(GameState& game) {
    game.getEntityController().spawnEntity<Entities::SpikeController>();
}
void DotDotDot::tick() {

}
void DotDotDot::handle(Client&, Packet&) {

}
void DotDotDot::left(Client&) {

}

DisasterServer::MapProperties DotDotDot::getMapTime() const {
    return MapProperties(3.42 * TICKS_PER_SEC, 20, 5);
}