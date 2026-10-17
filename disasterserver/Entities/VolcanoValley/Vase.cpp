#include "Vase.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Vase::Vase(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t vid) :
    Entity(id, server, state, "vase", position), vid(vid), rings(server.getRandom().nextInt(0, 4)) {}

Vase::~Vase() = default;

bool Vase::init() {
    return Entity::init();
}

bool Vase::tick() {
    return Entity::tick();
}

bool Vase::uninit() {
    return Entity::uninit();
}
