#include "EggTracker.hpp"

#include "Util/Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

EggTracker::EggTracker(entityId id, Server &server, GameState &state, const Vector2 &position) :
    Entity(id, server, state, "eggtrack", position) {}

EggTracker::~EggTracker() = default;

bool EggTracker::init() {
    Packet pack(PacketType::SERVER_ETRACKER_STATE);
    pack.write<uint8_t>(0);
    pack.write<entityId>(id);
    pack.writeVector2(position);
    pack.sendBroadcast(server);
    return true;
}

bool EggTracker::uninit() {
    Packet pack(PacketType::SERVER_ETRACKER_STATE);
    pack.write<uint8_t>(1);
    pack.write<entityId>(id);
    pack.write<clientId>(activId);
    pack.sendBroadcast(server);
    return true;
}
