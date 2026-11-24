#include "BlackRing.hpp"

#include "Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

BlackRing::BlackRing(entityId id, Server &server, GameState &state, const Vector2 &position) :
    Entity(id, server, state, "bring", position) {}

BlackRing::~BlackRing() = default;

bool BlackRing::create() {
    if (position == Vector2(mapBring, mapBring)) {
        Packet pack(PacketType::SERVER_BRING_STATE);
        pack.write<uint8_t>(0);
        pack.write<entityId>(id);
        pack.sendBroadcast(server);

        return Entity::create();
    }

    Packet pack(PacketType::SERVER_ERECTOR_BRING_SPAWN);
    pack.write<entityId>(id);
    pack.writeVector2(position);
    pack.sendBroadcast(server);

    return Entity::create();
}

bool BlackRing::destroy() {
    Packet pack(PacketType::SERVER_BRING_STATE);
    pack.write<uint8_t>(1);
    pack.write<entityId>(id);
    pack.sendBroadcast(server);

    return Entity::destroy();
}
