#include "Stalactits.hpp"

#include "Packet.hpp"
#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Stalactits::Stalactits(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t sid)
    : Entity(id, server, state, "dttits", position), sid(sid) {}

Stalactits::~Stalactits() = default;

bool Stalactits::init() {

    Packet pack(PacketType::SERVER_DTASS_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(sid);
    pack.writeVector2(position);
    pack.sendBroadcast(server, true);

    return true;
}

bool Stalactits::tick() {

    if (state) {
        vel += 0.164f * server.getDelta();
        position.y += vel * server.getDelta();

        Packet pack(PacketType::SERVER_DTASS_STATE);
        pack.write<uint8_t>(sid);
        pack.writeVector2(position);
        pack.sendBroadcast(server, false);
    } else {
        if (!show) {
            if (timer > TICKS_PER_SEC) {
                timer -= server.getDelta();
            }

            if (timer <= TICKS_PER_SEC) {
                Packet pack(PacketType::SERVER_DTASS_STATE);
            }
        }
    }

    return true;
}

bool Stalactits::uninit() {
    return true;
}

bool Stalactits::activate() {
    return true;
}
