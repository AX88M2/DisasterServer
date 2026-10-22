#include "HDDoor.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

HDDoor::HDDoor(entityId id, Server &server, GameState &state, const Vector2 &position)
    : Entity(id, server, state, "id", position){}

HDDoor::~HDDoor() {
}

bool HDDoor::init() {
    return Entity::init();
}

bool HDDoor::tick() {
    if (timer > 0) {
        timer -= server.getDelta();
        if (timer <= 0) {
            Packet pack(PacketType::SERVER_HDDOOR_STATE);
            pack.write<uint8_t>(1);
            pack.write<uint8_t>(1);
            pack.sendBroadcast(server, true);
        }
    }

    return Entity::tick();
}

bool HDDoor::uninit() {
    return Entity::uninit();
}

void HDDoor::toggle() {
    if (timer > 0) {
        return;
    }

    state = !state;
    timer = 10.0 * TICKS_PER_SEC;

    Packet pack(PacketType::SERVER_HDDOOR_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(state);
    pack.sendBroadcast(server, true);

    Packet packet(PacketType::SERVER_HDDOOR_STATE);
    packet.write<uint8_t>(1);
    packet.write<uint8_t>(0);
    packet.sendBroadcast(server, true);
}
