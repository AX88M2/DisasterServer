#include "TailsProjectile.hpp"
#include "Server.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;

TProjectile::TProjectile(entityId id, Server &server, GameState &state, const Vector2 &pos, uint16_t owner, int8_t dir, uint8_t exe, uint8_t charge, uint8_t damage) : Entity(id, server, state, "tproj", pos), owner(owner), dir(dir), exe(exe), charge(charge), damage(damage) {}

bool TProjectile::init() {
    Packet pack(PacketType::SERVER_TPROJECTILE_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint16_t>(static_cast<uint16_t>(position.x));
    pack.write<uint16_t>(static_cast<uint16_t>(position.y));
    pack.write<uint16_t>(owner);
    pack.write<uint8_t>(static_cast<uint8_t>(dir));
    pack.write<uint8_t>(damage);
    pack.write<uint8_t>(exe);
    pack.write<uint8_t>(charge);
    pack.sendBroadcast(server, true);
    return true;
}

bool TProjectile::tick() {
    if (timer <= 0)
        return false;

    if (state.getCurrentMapId() != 13) {
        if (position.x <= 0)
            return false;
    } else {
        if (position.x < 0)
            position.x = 3648;
        else if (position.x > 3648)
            position.x = 0;
    }

    Packet pack(PacketType::SERVER_TPROJECTILE_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint16_t>(static_cast<uint16_t>(position.x));
    pack.write<uint16_t>(static_cast<uint16_t>(position.y));
    pack.sendBroadcast(server, false);

    position.x += static_cast<float>(dir * 14 * server.getDelta());
    timer -= server.getDelta();
    return true;
}

bool TProjectile::uninit() {
    Packet pack(PacketType::SERVER_TPROJECTILE_STATE);
    pack.write<uint8_t>(2);
    pack.sendBroadcast(server, true);
    return true;
}