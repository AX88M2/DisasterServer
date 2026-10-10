#include "NPController.hpp"
#include "Server.hpp"

#include <algorithm>

#include "States/GameState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Maps;

NPController::NPController(entityId id, Server& server, GameState& state, const Vector2& pos, const std::string& tag) : Entity(id, server, state, tag, pos) {
}

bool NPController::init() {
    return true;
}

bool NPController::uninit() {
    return true;
}

bool NPController::tick()
{
    if (game.getGameTime().remaining() <= TICKS_PER_SEC && !isPreparationStarted)
    {
        timer = 5 * TICKS_PER_SEC;
        state = State::Prepare;
        isPreparationStarted = true;
    }

    switch (state)
    {
        case State::None:
        {
            const int transitionDelaySeconds = game.getGameTime().remaining() < TICKS_PER_SEC ? 2 : 15;
            if (timer >= transitionDelaySeconds * TICKS_PER_SEC)
            {
                Packet pack(PacketType::SERVER_NPCONTROLLER_STATE);
                pack.write<uint8_t>(0);
                pack.write<uint8_t>(0);
                pack.write<uint8_t>(0);
                pack.sendBroadcast(server, true);

                state = State::Prepare;
                timer = 0.0;
            }
            break;
        }

        case State::Prepare:
        {
            const int prepareDelaySeconds = game.getGameTime().remaining() < TICKS_PER_SEC ? 3 : 5;
            if (timer >= prepareDelaySeconds * TICKS_PER_SEC)
            {
                stage++;

                Packet pack(PacketType::SERVER_NPCONTROLLER_STATE);
                pack.write<uint8_t>(1);
                pack.write<uint8_t>(static_cast<uint8_t>(stage % 4));
                pack.write<uint8_t>(static_cast<uint8_t>(std::max<int>(stage - 1, 0) % 4));
                pack.sendBroadcast(server, true);

                state = State::None;
                timer = 0.0;
            }
            break;
        }
    }

    timer += server.getDelta();
    return true;
}