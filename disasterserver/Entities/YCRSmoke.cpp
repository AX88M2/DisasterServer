#include "YCRSmoke.hpp"

#include "Core/Constansts.hpp"
#include "States/GameState.hpp"
#include "Packet.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

YCRController::YCRController(entityId id, Server &server, GameState &state, const Vector2 &pos) : Entity(id, server, state, "ycrctrl", pos) {}

bool YCRController::tick() {
    switch (state) {
        case State::NONE: {
            if (timer >= TICKS_PER_SEC) {
                Packet pack(PacketType::SERVER_YCRSMOKE_READY);
                pack.write<uint8_t>(smokeId);
                pack.sendBroadcast(server, true);

                state = State::SOME;
            }
            break;
        }

        case State::SOME: {
            if (timer >= 6 * TICKS_PER_SEC) {
                state = State::NONE;
                timer = 0;
                activated = !activated;

                if (activated)
                    smokeId = static_cast<uint8_t>(std::rand() % 7);
                else
                    smokeId = 0;

                Packet pack(PacketType::SERVER_YCRSMOKE_STATE);
                pack.write<uint8_t>(activated);
                pack.write<uint8_t>(smokeId);
                pack.sendBroadcast(server, true);
            }
            break;
        }
    }

    timer += server.getDelta();
    return true;
}