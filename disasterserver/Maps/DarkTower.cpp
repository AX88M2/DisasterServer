#include "DarkTower.hpp"

#include "Client.hpp"
#include "Packet.hpp"
#include "States/GameState.hpp"
#include "Entities/DarkTower/DTBall.hpp"
#include "Entities/DarkTower/Stalactits.hpp"
#include "Entities/DarkTower/TailsDoll.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

DarkTower::DarkTower(MapContext context) : Map(context, "Dark Tower") {
}

void DarkTower::init() {
    state->getEntityController().spawnEntity<Entities::DTBall>();
    state->getEntityController().spawnEntity<Entities::TailsDoll>();

    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1744, 224), 0);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1840, 224), 1);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1936, 224), 2);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2032, 224), 3);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2128, 224), 4);

    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1824, 784), 5);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1920, 784), 6);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2016, 784), 7);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2112, 784), 8);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2208, 784), 9);

    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2464, 1384), 10);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2592, 1384), 11);

    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(3032, 64), 12);
    state->getEntityController().spawnEntity<Entities::Stalactits>(Vector2(3088, 64), 13);
}

void DarkTower::tick() {
}

void DarkTower::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_DTASS_ACTIVATE: {
            if (!client.isInGame()) {
                client.disconnect(DisconnectReason::OTHER, "AssertOrDisconnect({}) failed!", "client.isInGame()");
                return;
            }

            const uint8_t sid = packet.read<uint8_t>();

            auto* stal = state->getEntityController().findIf<Entities::Stalactits>([sid](Entities::Stalactits& b) {
                return b.getSID() == sid;
            });

            if (!stal) {
                return;
            }

            stal->activate();

            break;
        }
        default: break;
    }
}

void DarkTower::left(Client &client) {
}

MapProperties DarkTower::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(31)
        .timer(3.42 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}
