#include "LCEye.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

LCEye::LCEye(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t eyeId) : Entity(id, server, state, "lceye", pos), eyeId(eyeId) {}

bool LCEye::tick() {
    const double delta = server.getDelta();

    if (cooldownTimer.active()) {
        [[maybe_unused]]
        auto r = cooldownTimer.tick(delta);
        return true;
    }

    if (!chargeTimer.active()) {
        chargeTimer.start(1);
    }

    if (chargeTimer.tick(delta) == Countdown::TickResult::Second) {
        if (used && charge > 0) {
            charge -= 20;
            if (charge < 20) {
                cooldownTimer.start(2);
                used = false;
                chargeTimer.stop();
            }
            update();
        }
        else if (!used && charge < 100) {
            charge = std::min<uint8_t>(100, charge + 10);

            update();
        }
    }

    return true;
}

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