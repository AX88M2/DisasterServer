#include "NotPerfect.hpp"
#include "Server.hpp"
#include "../Entities/NPController.hpp"
#include "../States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

NotPerfect::NotPerfect(Server &server) : Map(server, "Not Perfect", 1, 59) {
}

NotPerfect::~NotPerfect() noexcept = default;

void NotPerfect::init(GameState &game) {
    game.getEntityController().spawnEntity<NPController>();
}

void NotPerfect::tick() {
}

void NotPerfect::handle(Client &client, Packet &packet) {
}

void NotPerfect::left(Client &client) {
}

MapProperties NotPerfect::getMapProperties() const {
    return MapProperties(2.585 * TICKS_PER_SEC, 20, 3);
}