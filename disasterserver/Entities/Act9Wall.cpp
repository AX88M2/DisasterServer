#include "Act9Wall.hpp"

#include "States/GameState.hpp"

constexpr int ACT9_ROOM_WIDTH = 3072;

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Act9Wall::Act9Wall(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t wid)
    : Entity(id, server, state, "act9wall", position), wid(wid) {}

Act9Wall::~Act9Wall() = default;

bool Act9Wall::init() {
    startTime = state.getGameTime().remaining() * TICKS_PER_SEC + state.getGameTime().remaining();
    return true;
}

bool Act9Wall::tick() {
    double time = state.getGameTime().remaining() * TICKS_PER_SEC + state.getGameTime().remaining();
    double off = (startTime - time) / startTime;

    position.x = position.x * off;
    position.y = position.y * off;

    Vector2 size;

    switch(wid) {
        case 0: {
            position.x = -2240;
            position.y = position.y - 768;
            size = Vector2(position.x + (64 * 117), position.y + (64 * 12));
            break;
        }

        case 1: {
            position.x = position.x - 2240;
            size = Vector2(position.x + (64 * 34), position.y + (64 * 19.5));
            break;
        }

        case 2: {
            position.x = (ACT9_ROOM_WIDTH - position.x) + 64;
            size = Vector2(position.x + (64 * 34), position.y + (64 * 19.5));
            break;
        }
        default: {
            Error("Invalid wall id!");
            return false;
        }
    }

    Packet pack(PacketType::SERVER_ACT9WALL_STATE);
    pack.write<uint8_t>(wid);
    pack.writeVector2(position);
    pack.sendBroadcast(server, false);

    for (auto &client : server.getClients()) {
        auto &player = client->getPlayer();

        if (!client->isInGame()) continue;
        if (player.getPosition() == Vector2(0, 0)) continue;
        if (player.isFlag(Player::Flags::PLAYER_DEAD)) continue;

        if (player.getPosition().x >= position.x && player.getPosition().y >= position.y && player.getPosition().x <= size.x && player.getPosition().y <= size.y) {
            client->disconnect(DisconnectReason::OTHER, "Inside the wall for too long!");
        }

    }

    return true;
}
