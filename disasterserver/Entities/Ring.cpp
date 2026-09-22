#include "Ring.hpp"

#include "Util/Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Ring::Ring(entityId id, Server &server, GameState &state, const Vector2 &position, const bool red) :
    Entity(id, server, state, "cring", position), red(red) {}

Ring::~Ring() = default;

bool Ring::init() {

    Packet pack(PacketType::SERVER_RING_STATE);
    pack.write<uint8_t>(2);
    pack.writeVector2(position);
    pack.write<uint8_t>(rid);
    pack.write<entityId>(id);
    pack.write<uint8_t>(red);
    pack.sendBroadcast(server);

    return true;
}

bool Ring::uninit() {

    Packet pack(PacketType::SERVER_RING_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint8_t>(rid);
    pack.write<entityId>(id);
    pack.sendBroadcast(server);

    return true;
}
