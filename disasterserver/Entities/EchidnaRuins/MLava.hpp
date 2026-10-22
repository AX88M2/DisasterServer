#pragma once

#include "Core/Constansts.hpp"
#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class MLava : public Entity {
        enum class States : uint8_t {
            IDLE,
            MOVEDOWN,
            RAISE,
            MOVE,
            LOWER
        };

        States state = States::IDLE;
        double timer = 5 * TICKS_PER_SEC;
        float start;
        float dist;
        float vel = 0;
    public:
        MLava(entityId id, Server &server, GameState &state, const Vector2 &position, float start, float dist);
        ~MLava() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}
