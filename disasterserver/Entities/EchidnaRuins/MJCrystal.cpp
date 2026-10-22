#include "MJCrystal.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

MJCrystal::MJCrystal(entityId id, Server &server, GameState &state, const Vector2 &position)
    : Entity(id, server, state, "mass", position) {}

MJCrystal::~MJCrystal() = default;

bool MJCrystal::init() {
    return Entity::init();
}

bool MJCrystal::tick() {

    if (timer >= nextTime) {
        state = !state;
        timer = 0;

        Packet pack(PacketType::SERVER_MJCRYSTAL_STATE);
        pack.write<uint8_t>(state);
        pack.sendBroadcast(server, true);

        nextTime = (10 + server.getRandom().nextInt(0, 5)) * TICKS_PER_SEC;
    }

    timer += server.getDelta();

    return Entity::tick();
}

bool MJCrystal::uninit() {
    return Entity::uninit();
}
