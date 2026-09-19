#include <cstdlib>

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Util/Packet.hpp"
#include "Core/Defines.hpp"
#include "Ring.hpp"

namespace DisasterServer {

bool Ring::init(Server& server) {
    auto* game = server.getStateController().getGameState();
    if (!game) return false;

    auto* map = game->getCurrentMap();
    if (!map) return false;

    const int ringCount = map->getRingCount();

    int active = 0;
    for (int i = 0; i < ringCount; ++i)
        if (game->isRingSlotUsed(i)) ++active;

    if (active >= ringCount)
        return false;

    int slot;
    do {
        slot = std::rand() % ringCount;
    } while (game->isRingSlotUsed(slot));

    game->setRingSlot(slot, true);
    rid = static_cast<uint8_t>(slot);
    red = map->getSpawnRedRings() && (std::rand() % 100 <= 10);

    Packet pack(PacketType::SERVER_RING_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(rid);
    pack.write<uint16_t>(id);
    pack.write<uint8_t>(red);
    pack.sendBroadcast(server, true);

    return true;
}

bool Ring::uninit(Server& server) {
    auto* game = server.getStateController().getGameState();
    if (game) {
        game->setRingSlot(rid, false);
    }

    Packet pack(PacketType::SERVER_RING_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint8_t>(rid);
    pack.write<uint16_t>(id);
    pack.sendBroadcast(server, true);

    return true;
}

} // namespace DisasterServer