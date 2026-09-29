#include "Exetior.hpp"

#include "States/GameState.hpp"
#include "Client.hpp"
#include "Entities/BlackRing.hpp"
#include "Entities/Ring.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Characters;

Exetior::Exetior(Server &server, Client &client) : Character(server, client, client.getPlayer(), "Exetior", true) {
}

Exetior::~Exetior() = default;

void Exetior::tick() {
}

bool Exetior::handle(GameState &state, Packet &packet) {
    auto &entityController = state.getEntityController();

    switch (packet.getType()) {
        case PacketType::CLIENT_ERECTOR_BRING_SPAWN: {
            AssertOrDisconnect(client, client.isInGame());
            AssertOrDisconnect(client, client.getId() == state.getExe());

            const Vector2 position = packet.readVector2();

            if (client.isModified()) {
                entityController.spawnEntity<Entities::Ring>(position);
                break;
            }

            entityController.spawnEntity<Entities::BlackRing>(position);
            break;
        }
        default: break;
    }
    return true;
}