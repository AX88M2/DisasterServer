#include "DarkTower.hpp"

#include "Client.hpp"
#include "Packet.hpp"
#include "States/GameState.hpp"
#include "Entities/DarkTower/DTBall.hpp"
#include "Entities/DarkTower/Stalactits.hpp"
#include "Entities/DarkTower/TailsDoll.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

DarkTower::DarkTower(Server &server) : Map(server, "Dark Tower", 1, 31) {
}

void DarkTower::init(GameState &game) {
    gameCtx = &game;

    game.getEntityController().spawnEntity<Entities::DTBall>();
    game.getEntityController().spawnEntity<Entities::TailsDoll>();

    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1744, 224), 0);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1840, 224), 1);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1936, 224), 2);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2032, 224), 3);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2128, 224), 4);

    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1824, 784), 5);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(1920, 784), 6);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2016, 784), 7);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2112, 784), 8);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2208, 784), 9);

    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2464, 1384), 10);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(2592, 1384), 11);

    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(3032, 64), 12);
    game.getEntityController().spawnEntity<Entities::Stalactits>(Vector2(3088, 64), 13);
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

            auto* stal = gameCtx->getEntityController().findIf<Entities::Stalactits>([sid](Entities::Stalactits& b) {
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
    return MapProperties(3.42 * TICKS_PER_SEC, 20, 5);
}
