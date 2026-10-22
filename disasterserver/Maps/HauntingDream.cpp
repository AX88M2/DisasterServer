#include "HauntingDream.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Entities/HauntingDream/HDDoor.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

HauntingDream::HauntingDream(Server &server) : Map(server, "Haunting Dream", 1, 31) {
}

void HauntingDream::init(GameState &game) {
    gameCtx = &game;
    game.getEntityController().spawnEntity<Entities::HDDoor>();
}

void HauntingDream::tick() {
}

void HauntingDream::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_HDDOOR_TOGGLE: {
            if (!client.isInGame()) {
                client.disconnect(DisconnectReason::OTHER, "AssertOrDisconnect({}) failed!", "client.isInGame()");
                return;
            }

            auto *door = gameCtx->getEntityController().findIf<Entities::HDDoor>([](Entities::HDDoor &) {
                return true;
            });

            if (!door) {
                return;
            }

            door->toggle();
            break;
        }
        default: break;
    }
}

void HauntingDream::left(Client &client) {
}

MapProperties HauntingDream::getMapProperties() const {
    return MapProperties(3.42 * TICKS_PER_SEC, 20, 5);
}
