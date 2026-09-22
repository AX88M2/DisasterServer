#include "EntityController.hpp"

using namespace DisasterServer;

EntityController::EntityController(Server &server, GameState &state) : server(server), state(state) {
}

void EntityController::tick() {
    for (auto &entity : entities) {
        entity->tick();
    }
}
