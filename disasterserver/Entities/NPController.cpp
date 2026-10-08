#include "NPController.hpp"
#include "../Server.hpp"

#include <algorithm>
#include <cmath>

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
    const double dt = server.getDelta();
    timeSec_ += dt;

    if (timeSec_ <= TICKS_PER_SEC && !balls_)
    {
        timer_ = 5 * TICKS_PER_SEC;
        state_ = State::Prepare;
        balls_ = true;
    }

    switch (state_)
    {
        case State::None:
        {
            const int intr1 = timeSec_ < TICKS_PER_SEC ? 2 : 15;
            if (timer_ >= intr1 * TICKS_PER_SEC)
            {
                Packet pack(PacketType::SERVER_NPCONTROLLER_STATE);
                pack.write<uint8_t>(0);
                pack.write<uint8_t>(0);
                pack.write<uint8_t>(0);
                server.broadcastEx(pack, true, 0);

                state_ = State::Prepare;
                timer_ = 0.0;
            }
            break;
        }

        case State::Prepare:
        {
            const int intr2 = timeSec_ < TICKS_PER_SEC ? 3 : 5;
            if (timer_ >= intr2 * TICKS_PER_SEC)
            {
                stage_++;

                Packet pack(PacketType::SERVER_NPCONTROLLER_STATE);
                pack.write<uint8_t>(1);
                pack.write<uint8_t>(uint8_t(stage_ % 4));
                pack.write<uint8_t>(uint8_t(std::max<int>(stage_ - 1, 0) % 4));
                server.broadcastEx(pack, true, 0);

                state_ = State::None;
                timer_ = 0.0;
            }
            break;
        }
    }

    timer_ += dt;
    return true;
}