#include "MysticWood.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/WDLatern.hpp"

using namespace DisasterServer::Maps;

WoodDream::WoodDream() : Map("Mystic Wood", 1, 30) {}

void WoodDream::init(GameState& game) {
    game.getEntityController().spawnEntity<Latern>();
    Debug("Spawned Latern");
}

void WoodDream::tick() {}
void WoodDream::handle(Client&, Packet&) {}
void WoodDream::left(Client&) {}

DisasterServer::MapProperties WoodDream::getMapTime() const {
    return MapProperties(static_cast<int>(2.585 * TICKS_PER_SEC), 20, 5);
}