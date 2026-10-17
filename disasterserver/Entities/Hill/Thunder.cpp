#include "Thunder.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Thunder::Thunder(entityId id, Server &server, GameState &state, const Vector2 &position) :
    Entity(id, server, state, "thunder", position), timer((15 + server.getRandom().nextInt(0, 5)) * TICKS_PER_SEC) {}

Thunder::~Thunder() = default;

bool Thunder::init() {
    return true;
}

bool Thunder::tick() {
    if (this->timer <= 2 * TICKS_PER_SEC && !this->flag) {
        Packet pack(PacketType::SERVER_GHZTHUNDER_STATE);
        pack.write<uint8_t>(0);
        pack.sendBroadcast(server, true);
        this->flag = true;
    } else if (this->timer <= 0) {
        Packet pack(PacketType::SERVER_GHZTHUNDER_STATE);
        pack.write<uint8_t>(1);
        pack.sendBroadcast(server, true);

        this->timer = (15 + server.getRandom().nextInt(0, 5)) * TICKS_PER_SEC;
        this->flag = false;
    }

    this->timer -= server.getDelta();
    return true;
}

bool Thunder::uninit() {
    return true;
}
