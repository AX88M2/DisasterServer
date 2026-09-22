#include "EntityController.hpp"

using namespace DisasterServer;

EntityController::EntityController(Server &server, GameState &state) : server(server), state(state) {
}

void EntityController::tick() {
    for (auto it = entities.begin(); it != entities.end(); ) {
        if (!(*it)->tick()) {
            (*it)->uninit();
            it = entities.erase(it);
        } else {
            ++it;
        }
    }
}