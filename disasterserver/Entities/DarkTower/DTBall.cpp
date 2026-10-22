#include "DTBall.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

DTBall::DTBall(entityId id, Server &server, GameState &state, const Vector2 &position)
    : Entity(id, server, state, "dtball", position){}

DTBall::~DTBall() = default;

bool DTBall::init() {
    return Entity::init();
}

bool DTBall::tick() {

    if (side) {
        state += 0.015 * server.getDelta();

        if (state >= 1) {
            side = 0;
        }
    } else {
        state -= 0.015 * server.getDelta();

        if (state <= -1) {
            side = 1;
        }
    }

    Packet pack(PacketType::SERVER_DTBALL_STATE);
    pack.write<float>(static_cast<float>(state));
    pack.sendBroadcast(server, false);

    return Entity::tick();
}

bool DTBall::uninit() {
    return Entity::uninit();
}
