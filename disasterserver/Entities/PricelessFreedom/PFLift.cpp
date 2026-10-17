#include "PFLift.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

PFLift::PFLift(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t lid, float start, float end) : Entity(id, server, state, "pflift", pos), lid(lid), start(start), end(end) {}

bool PFLift::init() {
    position.y = start;
    return true;
}

bool PFLift::tick() {
    const double delta = server.getDelta();

    if (!activated) {
        if (timer > 0) {
            timer -= delta;
            if (timer <= 0) {
                Packet pack(PacketType::SERVER_PFLIFT_STATE);
                pack.write<uint8_t>(3);
                pack.write<uint8_t>(lid);
                pack.write<uint16_t>(static_cast<uint16_t>(start));
                pack.sendBroadcast(server, true);
            }
        }
        return true;
    }

    if (position.y > end) {
        if (speed < 7.0f)
            speed += 0.052f * static_cast<float>(delta);

        position.y -= speed * static_cast<float>(delta);
    } else {
        Packet pack(PacketType::SERVER_PFLIFT_STATE);
        pack.write<uint8_t>(2);
        pack.write<uint8_t>(lid);
        pack.write<uint16_t>(activator);
        pack.write<uint16_t>(static_cast<uint16_t>(position.y));
        pack.sendBroadcast(server, true);

        timer = 1.5 * TICKS_PER_SEC;
        activated = false;
        activator = 0;
    }

    Packet pack(PacketType::SERVER_PFLIFT_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint8_t>(lid);
    pack.write<uint16_t>(activator);
    pack.write<uint16_t>(static_cast<uint16_t>(position.y));
    pack.sendBroadcast(server, false);

    return true;
}

bool PFLift::activate(clientId pid) {
    if (activated) return true;
    if (timer > 0) return true;

    activator  = static_cast<uint16_t>(pid);
    timer = 0;
    speed = 0;
    position.y = start;
    activated = true;

    Packet pack(PacketType::SERVER_PFLIFT_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(lid);
    pack.write<uint16_t>(activator);
    pack.sendBroadcast(server, true);

    return true;
}