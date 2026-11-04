#include "KindAndFair.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/KindAndFair/KAFSpeedBox.hpp"

using namespace DisasterServer::Maps;

KindAndFair::KindAndFair(MapContext context) : Map(context, "Kind And Fair") {}

void KindAndFair::init() {

    for (uint8_t i = 0; i < 11; i++) {
        state->getEntityController().spawnEntity<Entities::KafBox>(Vector2(), i);
    }

    Debug("Spawned 11 KafBox");
}

void KindAndFair::tick() {

}
void KindAndFair::left(Client&) {

}

void KindAndFair::handle(Client& client, Packet& packet) {
    if (packet.getType() != PacketType::CLIENT_KAFMONITOR_ACTIVATE) {
        return;
    }

    if (!client.isInGame()) {
        return;
    }

    const uint8_t nid  = packet.read<uint8_t>();
    const uint8_t proj = packet.read<uint8_t>();

    if (nid >= 11) {
        return;
    }

    auto* box = state->getEntityController().findIf<Entities::KafBox>([nid](Entities::KafBox& b) {
        return b.getNid() == nid;
    });

    if (!box) {
        return;
    }

    box->activate(client.getId(), proj);
}

DisasterServer::MapProperties KindAndFair::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .timer(3 * TICKS_PER_SEC)
        .multiplier(20)
        .ringCoff(5)
        .build();
}