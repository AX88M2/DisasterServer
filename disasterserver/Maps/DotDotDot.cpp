#include "DotDotDot.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Util/Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/Spike.hpp"

using namespace DisasterServer::Maps;

DotDotDot::DotDotDot() : Map("...", 1, 30) {}

void DotDotDot::init() {

}
void DotDotDot::tick() {

}
void DotDotDot::handle(Client&, Packet&) {

}
void DotDotDot::left(Client&) {

}

void DotDotDot::spawnControllers(GameState& game) {
    game.getEntityController().spawnEntity<SpikeController>();
}

DisasterServer::MapProperties DotDotDot::getMapTime() const {
    return MapProperties(205 * TICKS_PER_SEC, 20, 5);
}