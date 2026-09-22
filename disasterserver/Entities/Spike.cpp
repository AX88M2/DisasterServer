#include "Spike.hpp"
#include "Server.hpp"
#include "States/GameState.hpp"
#include "Util/Packet.hpp"

using namespace DisasterServer;

SpikeController::SpikeController(entityId id, Server &server, GameState &state, const Vector2 &pos)
    : Entity(id, server, state, "spikectrl", pos) {}

SpikeController::~SpikeController() = default;

bool SpikeController::tick() {
    timer -= server.getDelta();
    if (timer > 0)
        return true;

    if (++frame > 5)
        frame = 0;

    timer = (frame == 0 || frame == 2) ? 2.0 * TICKS_PER_SEC : 0.25;

    Packet pack(PacketType::SERVER_MOVINGSPIKE_STATE);
    pack.write<uint8_t>(frame);
    pack.sendBroadcast(server, true);

    return true;
}
