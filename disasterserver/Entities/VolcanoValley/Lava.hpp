#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Lava : public Entity {
        enum class State : uint8_t {
            IDLE,
            MOVEDOWN,
            RAISE,
            MOVE,
            LOWER
        };

        State state = State::IDLE;
        uint8_t lid;
        double timer;
        float start = 0;
        float dist = 0;
        float vel = 0;
    public:
        Lava(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t lid);
        ~Lava() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}
