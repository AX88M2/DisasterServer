#include "Act9.hpp"

#include "Entities/Act9Wall.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

Act9::Act9(Server &server) : Map(server, "Act 9", 1, 38) {
}

void Act9::init(GameState &game) {
    //game.getEntityController().spawnEntity<Entities::Act9Wall>(Vector2(0, 1025), 0);
    //game.getEntityController().spawnEntity<Entities::Act9Wall>(Vector2(1663, 0), 1);
    //game.getEntityController().spawnEntity<Entities::Act9Wall>(Vector2(1663, 0), 2);
}

void Act9::tick() {
}

void Act9::handle(Client &client, Packet &packet) {
}

void Act9::left(Client &client) {
}

MapProperties Act9::getMapProperties() const {
    return MapProperties(2.17 * TICKS_PER_SEC, 10, 3);
}
