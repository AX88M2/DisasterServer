#include "MapRing.hpp"

#include "Server.hpp"
#include "States/GameState.hpp"
#include "Util/Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

MapRing::MapRing(entityId id, Server &server, GameState &state, const Vector2 &position) :
    Entity(id, server, state, "ring", position) {}

MapRing::~MapRing() = default;

bool MapRing::init() {
    Map* map = state.getCurrentMap();
    if (!map) return false;

    const int ringCount = map->getRingCount();

    int active = 0;
    for (int i = 0; i < ringCount; ++i) {
        if (state.isRingSlotUsed(i)) {
            ++active;
        }
    }

    if (active >= ringCount)
        return false;

    int slot;

    do {
        slot = std::rand() % ringCount;
    } while (state.isRingSlotUsed(slot));

    state.setRingSlot(slot, true);
    rid = static_cast<uint8_t>(slot);
    red = map->getSpawnRedRings() && (std::rand() % 100 <= 10);

    Packet pack(PacketType::SERVER_RING_STATE);
    pack.write<uint8_t>(0);
    pack.write<uint8_t>(rid);
    pack.write<entityId>(id);
    pack.write<uint8_t>(red);
    pack.sendBroadcast(server, true);

    return true;
}

bool MapRing::uninit() {
    state.setRingSlot(rid, false);

    Packet pack(PacketType::SERVER_RING_STATE);
    pack.write<uint8_t>(1);
    pack.write<uint8_t>(rid);
    pack.write<entityId>(id);
    pack.sendBroadcast(server, true);

    return true;
}