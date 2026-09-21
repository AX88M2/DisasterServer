#include "Server.hpp"
#include "Controllers/StateController.hpp"
#include "States/GameState.hpp"
#include "Util/Packet.hpp"
#include "Ring.hpp"

using namespace DisasterServer;

Ring::Ring(uint16_t id, Server &server, StateController &stateController) : Entity(id, server, stateController, "ring") {}

bool Ring::init() {
    auto *state = stateController.getState<GameState>();

    if (!state) return false;

    Map* map = state->getCurrentMap();

    if (!map) return false;

    const int ringCount = map->getRingCount();

    int active = 0;
    for (int i = 0; i < ringCount; ++i)
        if (state->isRingSlotUsed(i)) ++active;

    if (active >= ringCount)
        return false;

    int slot;

    do {
        slot = std::rand() % ringCount;
    } while (state->isRingSlotUsed(slot));

    state->setRingSlot(slot, true);
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

bool Ring::uninit() {
    auto *state = stateController.getState<GameState>();
    if (state) {
        state->setRingSlot(rid, false);
    }

    Packet pack(PacketType::SERVER_RING_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint8_t>(rid);
    pack.write<uint16_t>(id);
    pack.sendBroadcast(server, true);

    return true;
}