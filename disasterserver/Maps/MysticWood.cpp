#include "MysticWood.hpp"

#include "Server.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/WDLatern.hpp"

using namespace DisasterServer::Maps;

MysticWood::MysticWood(Server &server) : Map(server, "Mystic Wood", 1, 30) {}

void MysticWood::init(GameState& game) {
    game.getEntityController().spawnEntity<Entities::Latern>();
    Debug("Spawned Latern");
}

void MysticWood::tick() {}
void MysticWood::handle(Client&, Packet&) {}
void MysticWood::left(Client&) {}

DisasterServer::MapProperties MysticWood::getMapProperties() const {
    return MapProperties(static_cast<int>(2.585 * TICKS_PER_SEC), 20, 5);
}