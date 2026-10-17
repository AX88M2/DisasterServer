#include "Hill.hpp"

#include "Entities/Hill/Thunder.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

Hill::Hill(Server &server) : Map(server, "Hill", 1, 26) {
}

void Hill::init(GameState &game) {
    game.getEntityController().spawnEntity<Entities::Thunder>();
}

void Hill::tick() {
}

void Hill::handle(Client &client, Packet &packet) {
}

void Hill::left(Client &client) {
}

MapProperties Hill::getMapProperties() const {
    return MapProperties();
}
