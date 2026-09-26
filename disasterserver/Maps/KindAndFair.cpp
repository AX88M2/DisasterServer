#include "KindAndFair.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/KAFSpeedBox.hpp"

using namespace DisasterServer::Maps;

KindAndFair::KindAndFair(Server &server) : Map(server, "Kind And Fair", 1, 30) {}

void KindAndFair::init(GameState& state) {
    this->game = &state;

    auto& ec = state.getEntityController();
    for (uint8_t i = 0; i < 11; i++) {
        ec.spawnEntity<Entities::KafBox>({}, i);
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

    auto* box = game->getEntityController().findIf<Entities::KafBox>([nid](Entities::KafBox& b) {
        return b.getNid() == nid;
    });

    if (!box) {
        return;
    }

    box->activate(client.getId(), proj);
}

DisasterServer::MapProperties KindAndFair::getMapProperties() const {
    return MapProperties(3 * TICKS_PER_SEC, 20, 5);
}