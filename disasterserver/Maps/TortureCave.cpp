#include "TortureCave.hpp"

#include "States/GameState.hpp"
#include "Entities/TortureCave/Acid.hpp"

using namespace DisasterServer::Maps;

TortureCave::TortureCave(Server &server) : Map(server, "Torture Cave", 1, 27) {
}

void TortureCave::init(GameState &game) {
    game.getEntityController().spawnEntity<Entities::Acid>();
}

void TortureCave::tick() {
}

void TortureCave::handle(Client &client, Packet &packet) {
}

void TortureCave::left(Client &client) {
}

DisasterServer::MapProperties TortureCave::getMapProperties() const {
    return MapProperties(2.585 * TICKS_PER_SEC, 20, 5);
}
