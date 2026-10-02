#include "Eggman.hpp"

#include "States/GameState.hpp"
#include "Entities/EggTracker.hpp"
#include "Client.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Eggman::Eggman(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Eggman") {
}

Eggman::~Eggman() = default;

void Eggman::tick() {
}

bool Eggman::handle(GameState &state, Packet &packet) {
    auto &entityController = state.getEntityController();

    switch (packet.getType()) {
        case PacketType::CLIENT_ETRACKER: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, state.getExe() != client.getId());

            const Vector2 position = packet.readVector2();

            if (countdown.remaining() > 0) {
                client.disconnect(DisconnectReason::OTHER, "remaining_cooldown: {}", countdown.remaining());
                return false;
            }

            entityController.spawnEntity<Entities::EggTracker>(position);
            countdown.setRemaining(10);
            break;
        }
        default: break;
    }

    return true;
}