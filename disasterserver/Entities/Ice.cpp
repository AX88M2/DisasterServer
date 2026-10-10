#include "Ice.hpp"

#include "Packet.hpp"
#include "Server.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Ice::Ice(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t iId)
    : Entity(id, server, state, "ice", position), iid(iId) {}

Ice::~Ice() = default;

bool Ice::init() {
    return true;
}

bool Ice::tick() {
    if (!activated) {
        return true;
    }

    timer -= server.getDelta();

    if (timer <= 0) {
        timer = 15 * TICKS_PER_SEC;
        activated = false;

        Packet pack(PacketType::SERVER_NAPICE_STATE);
        pack.write<uint8_t>(1);
        pack.write<uint8_t>(iid);
        pack.sendBroadcast(server, true);
    }

    return true;
}

bool Ice::uninit() {
    return true;
}

bool Ice::activate() {
    if (activated) {
        return false;
    }

    timer = 15 * TICKS_PER_SEC;
    activated = true;

    Packet pack(PacketType::SERVER_NAPICE_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(iid);
    pack.sendBroadcast(server, true);

    return true;
}
