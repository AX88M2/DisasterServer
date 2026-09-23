#include "LimpCity.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/LCEye.hpp"
#include "Entities/LCChain.hpp"

using namespace DisasterServer::Maps;

LimpCity::LimpCity() : Map("Limp City", 1, 30) {}

void LimpCity::init(GameState& game) {
    auto& ec = game.getEntityController();
    ec.spawnEntity<LCEye>({}, 0);
    ec.spawnEntity<LCEye>({}, 1);
    ec.spawnEntity<LCChain>();
    Debug("LimpCity: spawned 2 LCEye and 1 LCChain");
}

void LimpCity::tick() {

}
void LimpCity::left(Client&) {

}

void LimpCity::handle(Client& client, Packet& packet) {
    if (packet.getType() != PacketType::CLIENT_LCEYE_REQUEST_ACTIVATE)
        return;

    if (!client.isInGame())
        return;

    const uint8_t val = packet.read<uint8_t>();
    const uint8_t nid = packet.read<uint8_t>();
    const uint8_t target = packet.read<uint8_t>();

    if (nid >= 2)
        return;

    auto* game = client.getStateController().getState<GameState>();
    if (!game) return;

    auto* eye = game->getEntityController().findIf<LCEye>([nid](const LCEye& e) { return e.eyeId == nid; });

    if (!eye)
        return;

    if (val) {
        if (eye->used)
            return;

        if (eye->charge < 20)
            return;

        eye->useId  = client.getId();
        eye->target = target;
        eye->used   = true;
        eye->timer  = 0;
        eye->update();
    } else {
        eye->used  = false;
        eye->timer = 0;
        eye->update();
    }
}

DisasterServer::MapProperties LimpCity::getMapTime() const {
    return MapProperties(static_cast<int>(2.585 * TICKS_PER_SEC), 20, 5);
}