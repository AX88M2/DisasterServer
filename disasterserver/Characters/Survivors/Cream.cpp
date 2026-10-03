#include "Cream.hpp"

#include "States/GameState.hpp"
#include "Entities/Ring.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Cream::Cream(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Cream") {
}

Cream::~Cream() = default;

void Cream::tick() {
    auto result = countdown.tick(server.getDelta());
}

bool Cream::handle(GameState &state, Packet &packet) {
    auto &entityController = state.getEntityController();
    switch (packet.getType()) {
        case PacketType::CLIENT_CREAM_SPAWN_RINGS: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, state.getExe() != client.getId());

            const Vector2 position = packet.readVector2();
            const uint8_t isRedRing = packet.read<uint8_t>();

            if (countdown.remaining() > 0) {
                client.disconnect(DisconnectReason::OTHER, "remaining_cooldown: {}", countdown.remaining());
                return false;
            }

            if (client.isModified()) {
                Packet pack(PacketType::SERVER_RING_COLLECTED);
                pack.write<uint8_t>(0);
                pack.write<entityId>(0);
                pack.write<uint8_t>(true);
                pack.write<uint8_t>(false);
                pack.send(client);
            }

            AssertOrDisconnect(client, position.distance(client.getPlayer().getPosition()) <= 40);

            static double PI = 0.0;
            if (PI == 0.0) {
                PI = acos(-1);
            }

            if (isRedRing) {
                for (auto &entity : entityController.getEntities()) {
                    if (entity->is<Entities::Ring>()) {
                        auto ring = entity->as<Entities::Ring>();
                        if (!ring->isRed()) {
                            continue;
                        }

                        AssertOrDisconnect(client, position.distance(ring->getPosition()) >= 150);
                    }
                }

                float posX[2] = { 25, -27 };
                float posY[2] = { 0, 0 };

                for (int i = 0; i < 2; i++) {
                    entityController.spawnEntity<Entities::Ring>(Vector2(position.x + posX[i], position.y + posY[i]), isRedRing);
                }
            } else {
                float posX[3] = { 26, 0, -27 };
                float posY[3] = { 0, -26, 0 };
                for (int i = 0; i < 3; i++) {
                    entityController.spawnEntity<Entities::Ring>(Vector2(position.x + posX[i], position.y + posY[i]), isRedRing);
                }
            }

            countdown.setRemaining(25);
            break;
        }
        default: break;
    }

    return true;
}