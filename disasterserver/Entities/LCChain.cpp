#include "LCChain.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

LCChain::LCChain(entityId id, Server &server, GameState &state, const Vector2 &pos) : Entity(id, server, state, "lcchain", pos) {}

bool LCChain::tick() {
    switch (state) {
        case State::NONE:
            if (timer >= 8 * TICKS_PER_SEC) {
                Packet pack(PacketType::SERVER_LCCHAIN_STATE);
                pack.write<uint8_t>(0);
                pack.sendBroadcast(server, true);

                timer = 0;
                state = State::PREPARE;
            }
            break;

        case State::PREPARE:
            if (timer >= 2 * TICKS_PER_SEC) {
                Packet pack(PacketType::SERVER_LCCHAIN_STATE);
                pack.write<uint8_t>(1);
                pack.sendBroadcast(server, true);

                timer = 0;
                state = State::ACTIVATE;
            }
            break;

        case State::ACTIVATE:
            if (timer >= 2 * TICKS_PER_SEC) {
                Packet pack(PacketType::SERVER_LCCHAIN_STATE);
                pack.write<uint8_t>(2);
                pack.sendBroadcast(server, true);

                timer = 0;
                state = State::NONE;
            }
            break;
    }

    timer += server.getDelta();
    return true;
}