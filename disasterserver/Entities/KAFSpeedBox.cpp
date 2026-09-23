#include "KAFSpeedBox.hpp"

#include <cstdlib>

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;

KafBox::KafBox(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t nid) : Entity(id, server, state, "kafbox", pos), nid(nid) {}

bool KafBox::init() {
    Packet pack(PacketType::SERVER_KAFMONITOR_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(nid);
    pack.sendBroadcast(server, true);
    return true;
}

bool KafBox::tick() {
    if (!activated)
        return true;

    if (timer > 0) {
        timer -= server.getDelta();
        return true;
    }

    Packet pack(PacketType::SERVER_KAFMONITOR_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint8_t>(nid);
    pack.sendBroadcast(server, true);

    activated = false;
    return true;
}

bool KafBox::activate(uint16_t pid, uint8_t isProj) {
    if (activated)
        return true;

    Packet pack(PacketType::SERVER_KAFMONITOR_STATE);
    pack.write<uint8_t>(2);
    pack.write<uint8_t>(nid);
    pack.write<uint16_t>(isProj ? 0 : pid);
    pack.sendBroadcast(server, true);

    activated = true;
    timer = (25.0 + std::rand() % 5) * TICKS_PER_SEC;
    return true;
}