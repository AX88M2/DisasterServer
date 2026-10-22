#include "FartZone.hpp"

#include "States/GameState.hpp"

#include "Entities/BlackRing.hpp"
#include "Entities/YouCantRun/SpikeController.hpp"
#include "Entities/DarkTower/DTBall.hpp"
#include "Entities/FartZone/Dummy.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

FartZone::FartZone(Server &server) : Map(server, "Fart Zone", 1, 15) {
}

void FartZone::init(GameState &game) {
    gameCtx = &game;

    for (int i = 0; i < 3; i++) {
        game.getEntityController().spawnEntity<Entities::BlackRing>(Vector2(Entities::mapBring, Entities::mapBring));
    }

    game.getEntityController().spawnEntity<Entities::Dummy>(Vector2(1616, 2608));
    game.getEntityController().spawnEntity<Entities::SpikeController>();
    game.getEntityController().spawnEntity<Entities::DTBall>();
}

void FartZone::tick() {
}

void FartZone::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {

        case PacketType::CLIENT_FART_PUSH: {
            if (!client.isInGame()) {
                client.disconnect(DisconnectReason::OTHER, "AssertOrDisconnect({}) failed!", "client.isInGame()");
                return;
            }

            const int8_t spd = packet.read<int8_t>();

            auto *dum = gameCtx->getEntityController().findIf<Entities::Dummy>([](Entities::Dummy&) {
                return true;
            });

            if (!dum) {
                return;
            }

            dum->activate(spd);
            break;
        }

        default: break;
    }
}

void FartZone::left(Client &client) {
}

MapProperties FartZone::getMapProperties() const {
    return MapProperties(4.27 * TICKS_PER_SEC, 20, 1);
}
