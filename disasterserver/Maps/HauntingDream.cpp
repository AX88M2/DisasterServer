#include "HauntingDream.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Entities/HauntingDream/HDDoor.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

HauntingDream::HauntingDream(MapContext context) : Map(context, "Haunting Dream") {
}

void HauntingDream::init() {
    state->getEntityController().spawnEntity<Entities::HDDoor>();
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

            auto *door = state->getEntityController().findIf<Entities::HDDoor>([](Entities::HDDoor &) {
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
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(31)
        .timer(3.42 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}
