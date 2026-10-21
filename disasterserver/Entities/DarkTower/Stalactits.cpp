#include "Stalactits.hpp"

#include "Packet.hpp"
#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Stalactits::Stalactits(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t sid)
    : Entity(id, server, state, "dttits", position), sid(sid)
{
    startPosition = Vector2(position);
}

Stalactits::~Stalactits() = default;

bool Stalactits::init() {

    Packet pack(PacketType::SERVER_DTASS_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(sid);
    pack.writeVector2(position);
    pack.sendBroadcast(server, true);

    return true;
}

bool Stalactits::tick() {

    if (state) {
        vel += 0.164 * static_cast<float>(server.getDelta());
        position.y += vel * static_cast<float>(server.getDelta());

        Packet pack(PacketType::SERVER_DTASS_STATE);
        pack.write<uint8_t>(sid);
        pack.writeVector2(position);
        pack.sendBroadcast(server, false);
    } else {
        if (!show) {
            if (timer > TICKS_PER_SEC) {
                timer -= server.getDelta();
            }

            if (timer <= TICKS_PER_SEC) {
                Packet pack(PacketType::SERVER_DTASS_STATE);
                pack.write<uint8_t>(0);
                pack.write<uint8_t>(sid);
                pack.writeVector2(position);
                pack.sendBroadcast(server, true);

                show = true;
            }
        } else {
            if (timer > 0) {
                timer -= server.getDelta();
            }
        }

        if (timer <= 0) {
            for (auto &client : server.getClients()) {
                if (!client->isInGame()) {
                    continue;
                }

                auto &player = client->getPlayer();

                if (player.isFlag(Player::Flags::PLAYER_DEAD)) {
                    continue;
                }

                float dist = player.getPosition().y - position.y;

                if (dist > 0 && dist <= 336 && player.getPosition().x >= position.x && player.getPosition().x <= position.x + 80) {
                    vel = 0;

                    Packet pack(PacketType::SERVER_DTASS_STATE);
                    pack.write<uint8_t>(2);
                    pack.write<uint8_t>(sid);
                    pack.sendBroadcast(server, true);
                    state = true;
                    break;
                }
            }
        }
    }

    return true;
}

bool Stalactits::uninit() {
    return true;
}

bool Stalactits::activate() {
    if (!state) {
        return true;
    }

    show = false;
    state = false;
    timer = 25.0 + server.getRandom().nextInt(0, 5) * TICKS_PER_SEC;
    position.y = startPosition.y;
    vel = 0;

    Packet pack(PacketType::SERVER_DTASS_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint8_t>(sid);
    pack.sendBroadcast(server, true);

    return true;
}
