#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Slug : public Entity {
        Vector2 sPosition = {};

        enum class State : uint8_t {
            NONERIGHT,
            NONELEFT,
            RINGRIGHT,
            RINGLEFT,
            REDRINGRIGHT,
            REDRINGLEFT
        };

        enum class Drop : uint8_t {
            NORING,
            RING,
            REDRING
        };

        State moveState = State::NONERIGHT;
        Drop ring = Drop::NORING;

    public:
        Slug(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~Slug() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        void face(bool side);
    };
}