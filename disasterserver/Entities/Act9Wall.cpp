#include "Act9Wall.hpp"

#include "States/GameState.hpp"

constexpr int ACT9_ROOM_WIDTH = 3072;

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Act9Wall::Act9Wall(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t wid)
    : Entity(id, server, state, "act9wall", position), wid(wid) {}

Act9Wall::~Act9Wall() = default;

bool Act9Wall::init() {
    startTime = state.getGameTime().remaining() * TICKS_PER_SEC + state.getGameTime().remainingTicks();
    return true;
}

bool Act9Wall::tick() {
    double time = state.getGameTime().remaining() * TICKS_PER_SEC + state.getGameTime().remainingTicks();
    double off = (startTime - time) / startTime;

    double x = position.x * off;
    double y = position.y * off;
    double wx = 0;
    double hy = 0;

    Packet pack(PacketType::SERVER_ACT9WALL_STATE);
    pack.write<uint8_t>(wid);
    pack.write<uint16_t>(static_cast<uint16_t>(x));
    pack.write<uint16_t>(static_cast<uint16_t>(y));
    pack.sendBroadcast(server, false);

    switch(wid) {
        case 0: {
            x = -2240;
            y = y - 768;
            wx = x + (64 * 117);
            hy = y + (64 * 12);
            break;
        }

        case 1: {
            x = x - 2240;
            wx = x + (64 * 34);
            hy = y + (64 * 19.5);
            break;
        }

        case 2: {
            x = (ACT9_ROOM_WIDTH - x) + 64;
            wx = x + (64 * 34);
            hy = x + (64 * 19.5);
            break;
        }
        default: {
            Error("Invalid wall id!");
            return false;
        }
    }

    for (auto &client : server.getClients()) {
        auto &player = client->getPlayer();

        if (!client->isInGame()) continue;
        if (player.getPosition() == Vector2(0, 0)) continue;
        if (player.isFlag(Player::Flags::PLAYER_DEAD)) continue;

        if (player.getPosition().x >= x && player.getPosition().y >= y && player.getPosition().x <= wx && player.getPosition().y <= hy) {
            client->disconnect(DisconnectReason::OTHER, "Inside the wall for too long!");
        }

    }

    return true;
}
