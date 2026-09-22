#include "BlackRing.hpp"

#include "Util/Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

#define MAP_BRING INT16_MAX

BlackRing::BlackRing(entityId id, Server &server, GameState &state) : Entity(id, server, state, "bring") {}
BlackRing::~BlackRing() noexcept = default;

bool BlackRing::init() {
    if (position == Vector2(MAP_BRING, MAP_BRING)) {
        Packet pack(PacketType::SERVER_BRING_STATE);
        pack.write<uint8_t>(0);
        pack.write<entityId>(id);
        pack.sendBroadcast(server);

        return true;
    }

    Packet pack(PacketType::SERVER_ERECTOR_BRING_SPAWN);
    pack.write<entityId>(id);
    pack.writeVector2(position);
    pack.sendBroadcast(server);

    return true;
}

bool BlackRing::uninit() {
    Packet pack(PacketType::SERVER_BRING_STATE);
    pack.write<uint8_t>(1);
    pack.write<entityId>(id);
    pack.sendBroadcast(server);

    return true;
}
