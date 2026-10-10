#include "NastyParadise.hpp"

#include "States/GameState.hpp"
#include "Entities/Ice.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

NastyParadise::NastyParadise(Server &server) : Map(server, "Nasty Paradise", 1, 26) {
}

void NastyParadise::init(GameState &game) {
    this->gameCtx = &game;

    for (uint8_t i = 0; i < 10; i++) {
        game.getEntityController().spawnEntity<Entities::Ice>(Vector2(), i);
    }
}

void NastyParadise::tick() {
}

void NastyParadise::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_NAPICE_ACTIVATE: {
            if (!client.isInGame()) {
                return;
            }

            const uint8_t iid = packet.read<uint8_t>();

            auto* ice = gameCtx->getEntityController().findIf<Entities::Ice>([iid](Entities::Ice& b) {
                return b.getIid() == iid;
            });

            if (!ice) {
                return;
            }

            ice->activate();

            break;
        }
        default: break;
    }
}

void NastyParadise::left(Client &client) {
}

MapProperties NastyParadise::getMapProperties() const {
    return MapProperties(2.585 * TICKS_PER_SEC, 20, 5);
}
