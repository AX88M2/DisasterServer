#include "Tails.hpp"

#include "States/GameState.hpp"

#include "Client.hpp"
#include "Entities/TailsProjectile.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;
using namespace DisasterServer::Characters;

Tails::Tails(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Tails") {
}

Tails::~Tails() = default;

void Tails::tick() {

    auto result = countdown.tick(server.getDelta());

    if (result == Countdown::TickResult::Finished) {
        Debug("Laser is charged!");
    }
}

void Tails::demonize() {
    countdown.setRemaining(0);
}

bool Tails::handle(GameState& state, Packet &packet) {
    auto &entityController = state.getEntityController();

    switch (packet.getType()) {
        case PacketType::CLIENT_TPROJECTILE: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() != state.getExe());

            int projCount = 0;
            for (auto& e : entityController.getEntities()) {
                if (e->is<TProjectile>()) {
                    projCount++;
                }
            }

            AssertOrDisconnect(client, projCount <= 2);

            const bool demonized = client.getPlayer().isFlag(Player::Flags::PLAYER_DEMONIZED);

            if (countdown.remaining() > 0) {
                client.disconnect(DisconnectReason::OTHER, "is_exe: {}, remaining_cooldown: {}", demonized, countdown.remaining());
                return false;
            }

            const Vector2 position = packet.readVector2();
            int8_t dir = packet.read<int8_t>();
            const uint8_t dmg = packet.read<uint8_t>();
            const uint8_t exe = packet.read<uint8_t>();
            const uint8_t chg = packet.read<uint8_t>();
            AssertOrDisconnect(client, dir >= -1 && dir <= 1);

            if (client.isModified()) {
                if (dir > 0) dir = -1;
                else if (dir < 0) dir = 1;
            }

            if (demonized) {
                AssertOrDisconnect(client, dmg <= 60);
            } else {
                AssertOrDisconnect(client, dmg <= 6);
            }

            entityController.spawnEntity<TProjectile>(position, client.getId(), dir, exe, chg, dmg);
            countdown.setRemaining(10 * TICKS_PER_SEC);
            break;
        }

        default: break;
    }

    return true;
}
