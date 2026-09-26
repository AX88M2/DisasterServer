#include "LimpCity.hpp"

#include "Server.hpp"
#include "Client.hpp"
#include "Packet.hpp"
#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Entities/LCEye.hpp"
#include "Entities/LCChain.hpp"

using namespace DisasterServer::Maps;

LimpCity::LimpCity(Server &server) : Map(server, "Limp City", 1, 30) {
}

void LimpCity::init(GameState& state) {
    this->game = &state;

    auto& ec = state.getEntityController();
    ec.spawnEntity<Entities::LCEye>({}, 0);
    ec.spawnEntity<Entities::LCEye>({}, 1);
    ec.spawnEntity<Entities::LCChain>();
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

    const uint8_t isActivated = packet.read<uint8_t>();
    const uint8_t eyeId = packet.read<uint8_t>();
    const uint8_t target = packet.read<uint8_t>();

    if (eyeId >= 2)
        return;

    auto* eye = game->getEntityController().findIf<Entities::LCEye>([eyeId](Entities::LCEye& e) {
        return e.getEyeId() == eyeId;
    });

    if (!eye)
        return;

    if (isActivated) {
        if (eye->isUsed())
            return;

        if (eye->getCharge() < 20)
            return;

        eye->setUseId(client.getId());
        eye->setTarget(target);
        eye->setUsed(true);
        eye->getChargeTimer().stop();
        eye->update();
    } else {
        eye->setUsed(false);
        eye->getChargeTimer().stop();
        eye->update();
    }
}

DisasterServer::MapProperties LimpCity::getMapProperties() const {
    return MapProperties(2.585 * TICKS_PER_SEC, 20, 5);
}