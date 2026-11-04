#include "FartZone.hpp"

#include "States/GameState.hpp"

#include "Entities/BlackRing.hpp"
#include "Entities/YouCantRun/SpikeController.hpp"
#include "Entities/DarkTower/DTBall.hpp"
#include "Entities/FartZone/Dummy.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

FartZone::FartZone(MapContext context) : Map(context, "Fart Zone") {
}

void FartZone::init() {
    for (int i = 0; i < 3; i++) {
        state->getEntityController().spawnEntity<Entities::BlackRing>(Vector2(Entities::mapBring, Entities::mapBring));
    }

    state->getEntityController().spawnEntity<Entities::Dummy>(Vector2(1616, 2608));
    state->getEntityController().spawnEntity<Entities::SpikeController>();
    state->getEntityController().spawnEntity<Entities::DTBall>();
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

            auto *dum = state->getEntityController().findIf<Entities::Dummy>([](Entities::Dummy&) {
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
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(15)
        .timer(4.27 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(1)
        .build();
}
