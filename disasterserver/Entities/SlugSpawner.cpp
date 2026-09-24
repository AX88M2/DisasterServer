#include "SlugSpawner.hpp"

#include "Server.hpp"
#include "Slug.hpp"
#include "States/GameState.hpp"
#include "Util/Random.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

SlugSpawner::SlugSpawner(entityId id, Server &server, GameState &state, const Vector2 &position) :
    Entity(id, server, state, "slugspawn", position) {}

SlugSpawner::~SlugSpawner() = default;

bool SlugSpawner::tick() {
    if (slug != nullptr) {
        return true;
    }

    if (offset > 0) {
        offset -= server.getDelta();
        return true;
    }

    timer += server.getDelta();
    if (timer >= 15 * TICKS_PER_SEC) {
        const auto entity = state.getEntityController().spawnEntity<Slug>(position);

        timer = 0;
        slug = entity;

        offset = static_cast<double>(Random::randInt() % 2 * TICKS_PER_SEC);
    }

    return true;
}
