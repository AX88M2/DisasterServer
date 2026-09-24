#include "EntityController.hpp"

using namespace DisasterServer;

EntityController::EntityController(Server &server, GameState &state) : server(server), state(state) {
}

void EntityController::tick() {
    for (auto it = entities.begin(); it != entities.end();) {
        Entity* entity = it->get();

        if (pendingRemoval.contains(entity->getId())) {
            ++it;
            continue;
        }

        if (!entity->tick()) {
            pendingRemoval.insert(entity->getId());
        }

        ++it;
    }

    for (auto& entity : entities) {
        if (pendingRemoval.contains(entity->getId())) {
            entity->uninit();
        }
    }

    std::erase_if(entities, [this](const std::unique_ptr<Entity>& entity) {
        return pendingRemoval.contains(entity->getId());
    });

    pendingRemoval.clear();

    for (auto& entity : pendingEntities) {
        entities.push_back(std::move(entity));
    }

    pendingEntities.clear();
}

bool EntityController::despawnEntity(entityId id) {
    if (pendingRemoval.contains(id)) {
        return false;
    }

    const auto it = std::ranges::find_if(entities, [id](const auto& entity) {
        return entity->getId() == id;
    });

    if (it == entities.end()) {
        return false;
    }

    pendingRemoval.insert(id);
    
    return true;
}
