#include "PricelessFreedom.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/PFLift.hpp"
#include "Entities/BlackRing.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

PricelessFreedom::PricelessFreedom(Server &server) : Map(server, "Priceless Freedom", 1, 30) {}

void PricelessFreedom::init(GameState& state) {
    this->game = &state;

    auto& ec = state.getEntityController();

    for (uint8_t i = 0; i < 29; i++) {
        ec.spawnEntity<Entities::BlackRing>({INT16_MAX, INT16_MAX});
    }

    ec.spawnEntity<Entities::PFLift>({}, 0, 1669.0f, 1016.0f);
    ec.spawnEntity<Entities::PFLift>({}, 1, 1069.0f,  704.0f);
    ec.spawnEntity<Entities::PFLift>({}, 2,  829.0f,  400.0f);
    ec.spawnEntity<Entities::PFLift>({}, 3, 1070.0f,  544.0f);
}

void PricelessFreedom::tick() {}
void PricelessFreedom::left(Client&) {}

void PricelessFreedom::handle(Client& client, Packet& packet) {
    if (packet.getType() != PacketType::CLIENT_PFLIT_ACTIVATE)
        return;

    if (!client.isInGame())
        return;

    const uint8_t lid = packet.read<uint8_t>();

    auto* lift = game->getEntityController().findIf<Entities::PFLift>([lid](Entities::PFLift& l) { return l.getLid() == lid; });

    if (!lift)
        return;

    lift->activate(client.getId());
}

DisasterServer::MapProperties PricelessFreedom::getMapProperties() const {
    return MapProperties(static_cast<int>(2.585 * TICKS_PER_SEC), 10, 5);
}