#include "EchidnaRuins.hpp"

#include "Entities/EchidnaRuins/MJCrystal.hpp"
#include "Entities/EchidnaRuins/MJew.hpp"
#include "Entities/EchidnaRuins/MLava.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

EchidnaRuins::EchidnaRuins(Server &server) : Map(server, "Echidna Ruins", 1, 28 + 1) {
}

void EchidnaRuins::init(GameState &game) {
    game.getEntityController().spawnEntity<Entities::MLava>(Vector2(), 1624, 512);
    game.getEntityController().spawnEntity<Entities::MJCrystal>();
    game.getEntityController().spawnEntity<Entities::MJew>();
}

void EchidnaRuins::tick() {
}

void EchidnaRuins::handle(Client &client, Packet &packet) {
}

void EchidnaRuins::left(Client &client) {
}

MapProperties EchidnaRuins::getMapProperties() const {
    return MapProperties(3.42 * TICKS_PER_SEC, 20, 5);
}
