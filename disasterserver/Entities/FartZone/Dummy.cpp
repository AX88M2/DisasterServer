#include "Dummy.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Dummy::Dummy(entityId id, Server &server, GameState &state, const Vector2 &position)
    : Entity(id, server, state, "dummy", position) {}

Dummy::~Dummy() = default;

bool Dummy::init() {
    return Entity::init();
}

bool Dummy::tick() {
    position.x += vel * static_cast<float>(server.getDelta());
    position.x = static_cast<float>(fmin(fmax(position.x, 1282), 2944));
    vel -= fmin(fabs(vel), 0.046875 * 4.) * Vector2::sign(vel) * static_cast<float>(server.getDelta());

    Packet pack(PacketType::SERVER_FART_STATE);
    pack.writeVector2(position);
    pack.sendBroadcast(server, false);

    return Entity::tick();
}

bool Dummy::uninit() {
    return Entity::uninit();
}

void Dummy::activate(int8_t dir) {
    vel = dir;
}
