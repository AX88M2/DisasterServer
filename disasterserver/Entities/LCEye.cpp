#include "LCEye.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;

LCEye::LCEye(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t eyeId) : Entity(id, server, state, "lceye", pos), eyeId(eyeId) {}

bool LCEye::update() {
    Packet pack(PacketType::SERVER_LCEYE_STATE);
    pack.write<uint8_t>(eyeId);
    pack.write<uint8_t>(used);
    pack.write<uint16_t>(useId);
    pack.write<uint8_t>(static_cast<uint8_t>(target));
    pack.write<uint8_t>(charge);
    pack.sendBroadcast(server, true);
    return true;
}

bool LCEye::tick() {
    if (cooldown > 0) {
        cooldown -= server.getDelta();
        return true;
    }

    if (timer >= TICKS_PER_SEC) {
        if (used && charge > 0) {
            charge -= 20;
            if (charge < 20) {
                cooldown = 2.0 * TICKS_PER_SEC;
                used = false;
                timer = 0;
            }
            update();
        } else if (!used && charge < 100) {
            charge += 10;
            update();
        }

        timer = 0;
    }

    timer += server.getDelta();
    return true;
}