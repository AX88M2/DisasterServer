#include "ExellerClone.hpp"

#include "Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

ExellerClone::ExellerClone(entityId id, Server &server, GameState &state, const Vector2 &position, int8_t dir, clientId owner) :
    Entity(id, server, state, "exclone", position), dir(dir), owner(owner) {}

ExellerClone::~ExellerClone() = default;

bool ExellerClone::init() {
    Packet pack(PacketType::SERVER_EXELLERCLONE_STATE);
    pack.write<uint8_t>(0);
    pack.write<entityId>(id);
    pack.write<clientId>(owner);
    pack.writeVector2(position);
    pack.write<int8_t>(dir);
    pack.sendBroadcast(server);
    return true;
}

bool ExellerClone::uninit() {
    return true;
}
