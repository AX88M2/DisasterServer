#include "Shard.hpp"

#include "Util/Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Shard::Shard(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t spawned) :
    Entity(id, server, state, "shard", position), spawned(spawned) {}

Shard::~Shard() = default;

bool Shard::init() {
    Packet pack(PacketType::SERVER_RMZSHARD_STATE);
    pack.write<uint8_t>(spawned);
    pack.write<entityId>(id);
    pack.writeVector2(position);
    pack.sendBroadcast(server);

    return true;
}
