#include "YouCantRun.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/Spike.hpp"
#include "Entities/YCRSmoke.hpp"

using namespace DisasterServer::Maps;

YouCantRun::YouCantRun(Server &server) : Map(server, "You Can't Run", 1, 30) {}

void YouCantRun::init(GameState& game) {
    auto& ec = game.getEntityController();
    ec.spawnEntity<Entities::SpikeController>();
    ec.spawnEntity<Entities::YCRController>();
    Debug("Spawned SpikeController and YCRController");
}

void YouCantRun::tick() {

}
void YouCantRun::handle(Client&, Packet&) {

}
void YouCantRun::left(Client&) {

}

DisasterServer::MapProperties YouCantRun::getMapProperties() const {
    return MapProperties(3 * TICKS_PER_SEC, 20, 5);
}