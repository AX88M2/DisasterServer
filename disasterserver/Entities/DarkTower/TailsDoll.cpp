#include "TailsDoll.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

std::vector spots = {
    Vector2(177, 944),
    Vector2(1953, 544),
    Vector2(3279, 224),
    Vector2(4101, 544),
    Vector2(4060, 1264),
    Vector2(3805, 1824),
    Vector2(2562, 1584),
    Vector2(515, 1824),
    Vector2(2115, 1056),
    Vector2(984, 1184),
    Vector2(1498, 1504)
};

TailsDoll::TailsDoll(entityId id, Server &server, GameState &ctx, const Vector2 &position)
    : Entity(id, server, ctx, "tdoll", position) {}

TailsDoll::~TailsDoll() = default;

bool TailsDoll::init() {
    findSpot();

    Packet pack(PacketType::SERVER_DTTAILSDOLL_STATE);
    pack.write<uint8_t>(0);
    pack.writeVector2(position);
    pack.write<States>(state);
    pack.sendBroadcast(server, true);

    return true;
}

bool TailsDoll::tick() {
    switch (state) {
        case States::NONE: {
            if (findTarget()) {
                timer = (1 + server.getRandom().nextInt(0, 2) * 0.5) * TICKS_PER_SEC;
                state = States::READY;

                Packet pack(PacketType::SERVER_DTTAILSDOLL_STATE);
                pack.write<uint8_t>(2);
                pack.send(**server.findClient(static_cast<clientId>(target)), true);
            }
            break;
        }

        case States::READY: {
            timer -= server.getDelta();

            if (timer <= 0) {
                Packet pack(PacketType::SERVER_DTTAILSDOLL_STATE);
                pack.write<uint8_t>(3);
                pack.send(**server.findClient(static_cast<clientId>(target)), true);

                state = States::FOLLOW;
            }
            break;
        }

        case States::FOLLOW: {
            findTarget();

            if (!isValidTarget()) {
                state = States::RELOC;
                break;
            }

            auto client = server.findClient(static_cast<clientId>(target));
            if (!client.has_value() && !(*client)->isInGame()) {
                state = States::RELOC;
                break;
            }

            auto &player = (*client)->getPlayer();

            if (abs((int)(player.getPosition().x - position.x)) >= 4) {
                vel.x += Vector2::sign((int)(player.getPosition().x - position.x)) * 0.512 * server.getDelta();
                vel.x = fmin(fmax(vel.x, -5), 5);
            }

            if (abs((int)(player.getPosition().y - position.y)) >= 5) {
                vel.y += Vector2::sign((int)(player.getPosition().y - position.y)) * 0.480 * server.getDelta();
                vel.y = fmin(fmax(vel.y, -5), 5);
            }

            position.x += vel.x * (float)server.getDelta();
            position.y += vel.y * (float)server.getDelta();

            if (position.distance(player.getPosition()) < 12) {
                Packet pack(PacketType::SERVER_DTTAILSDOLL_STATE);
                pack.write<uint8_t>(1);
                pack.send(**server.findClient(static_cast<clientId>(target)), true);

                state = States::RELOC;
                break;
            }

            break;
        }

        case States::RELOC: {
            findSpot();
            state = States::NONE;
            break;
        }

        default: break;
    }

    Packet pack(PacketType::SERVER_DTTAILSDOLL_STATE);
    pack.writeVector2(position);
    pack.write<States>(state);
    pack.sendBroadcast(server, false);

    return true;
}

bool TailsDoll::uninit() {
    return true;
}

void TailsDoll::findSpot() {
    std::vector<Vector2> newSpots;

    for (int i = 0; i < 11; i++) {
        bool canUse = true;

        for (auto &client: server.getClients()) {
            if (!client->isInGame()) {
                continue;
            }

            if (spots[i].distance(client->getPlayer().getPosition()) < 480.0f) {
                canUse = false;
                break;
            }
        }

        if (canUse) {
            newSpots.push_back(spots[i]);
        }
    }

    if (newSpots.empty()) {
        int randSpot = server.getRandom().nextInt(0, newSpots.size());

        Vector2 spot = newSpots[randSpot];

        position = Vector2(spot);
        Debug("Tails doll found spot {} at {} {}", randSpot, position.x, position.y);
    } else {
        Vector2 spot = spots[server.getRandom().nextInt(0, static_cast<int>(spots.size()) - 1)];
        position = Vector2(spot);

        Debug("Tails doll didn't find a spot, using {} {}", position.x, position.y);
    }
}

bool TailsDoll::isValidTarget() {
    if (target == -1) {
        return false;
    }

    for (auto &client : server.getClients()) {
        if (!client->isInGame()) {
            continue;
        }

        auto &player = client->getPlayer();

        if (client->getId() == game.getExe()) {
            continue;
        }

        if (player.isFlag(Player::Flags::PLAYER_DEAD)) {
            continue;
        }

        if (player.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
            continue;
        }

        if (player.isFlag(Player::Flags::PLAYER_ESCAPED)) {
            continue;
        }

        if (client->getId() == target) {
            return true;
        }
    }

    return false;
}

bool TailsDoll::findTarget() {
    for (auto &client : server.getClients()) {
        if (!client->isInGame()) {
            continue;
        }

        auto &player = client->getPlayer();

        if (client->getId() == game.getExe()) {
            continue;
        }

        if (player.isFlag(Player::Flags::PLAYER_DEAD)) {
            continue;
        }

        if (player.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
            continue;
        }

        if (player.isFlag(Player::Flags::PLAYER_ESCAPED)) {
            continue;
        }

        if (position.distance(player.getPosition()) < 130) {
            target = client->getId();
            return true;
        }
    }

    return false;
}
