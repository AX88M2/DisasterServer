#include "PricelessFreedom.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "States/GameState.hpp"
#include "Entities/PricelessFreedom/PFLift.hpp"
#include "Entities/BlackRing.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

PricelessFreedom::PricelessFreedom(MapContext context) : Map(context, "Priceless Freedom") {}

void PricelessFreedom::init() {

    for (uint8_t i = 0; i < 29; i++) {
        state->getEntityController().spawnEntity<Entities::BlackRing>(Vector2(INT16_MAX, INT16_MAX));
    }

    state->getEntityController().spawnEntity<Entities::PFLift>(Vector2(), 0, 1669.0f, 1016.0f);
    state->getEntityController().spawnEntity<Entities::PFLift>(Vector2(), 1, 1069.0f,  704.0f);
    state->getEntityController().spawnEntity<Entities::PFLift>(Vector2(), 2,  829.0f,  400.0f);
    state->getEntityController().spawnEntity<Entities::PFLift>(Vector2(), 3, 1070.0f,  544.0f);
}

void PricelessFreedom::tick() {}
void PricelessFreedom::left(Client&) {}

void PricelessFreedom::handle(Client& client, Packet& packet) {
    if (packet.getType() != PacketType::CLIENT_PFLIT_ACTIVATE)
        return;

    if (!client.isInGame())
        return;

    const uint8_t lid = packet.read<uint8_t>();

    auto* lift = state->getEntityController().findIf<Entities::PFLift>([lid](Entities::PFLift& l) {
        return l.getLid() == lid;
    });

    if (!lift)
        return;

    lift->activate(client.getId());
}

MapProperties PricelessFreedom::getMapProperties() const {
    return MapPropertiesBuilder()
        .redRings(1)
        .ringCount(30)
        .timer(2.585 * TICKS_PER_SEC)
        .multiplier(10)
        .ringCoff(5)
        .build();
}