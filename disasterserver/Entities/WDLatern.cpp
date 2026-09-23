#include "WDLatern.hpp"

#include <cstdlib>

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;

Latern::Latern(entityId id, Server &server, GameState &state, const Vector2 &pos) : Entity(id, server, state, "latrn", pos), time(static_cast<uint16_t>(7 + std::rand() % 2)) {}

bool Latern::tick() {
    timer += server.getDelta();

    if (!side) {
        if (timer >= time * TICKS_PER_SEC) {
            lid = static_cast<uint8_t>(std::rand() % 7);

            Packet pack(PacketType::SERVER_WDLATERN_ACTIVATE);
            pack.write<uint8_t>(1);
            pack.write<uint8_t>(lid);
            pack.sendBroadcast(server, true);

            side  = true;
            timer = 0;
            time  = static_cast<uint16_t>(20 + std::rand() % 2);
        }
    } else {
        if (timer >= time * TICKS_PER_SEC) {
            Packet pack(PacketType::SERVER_WDLATERN_ACTIVATE);
            pack.write<uint8_t>(0);
            pack.write<uint8_t>(lid);
            pack.sendBroadcast(server, true);

            side  = false;
            timer = 0;
            time  = static_cast<uint16_t>(7 + std::rand() % 2);
        }
    }

    return true;
}