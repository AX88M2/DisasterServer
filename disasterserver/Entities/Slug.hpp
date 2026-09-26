#pragma once

#include <ranges>

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Slug : public Entity {
    public:


        enum class Drop : uint8_t {
            NORING,
            RING,
            REDRING
        };

    private:
        enum class State : uint8_t {
            NONERIGHT,
            NONELEFT,
            RINGRIGHT,
            RINGLEFT,
            REDRINGRIGHT,
            REDRINGLEFT
        };


        Vector2 sPosition = {};

        State moveState = State::NONERIGHT;
        Drop drop = Drop::NORING;

    public:
        Slug(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~Slug() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        Drop getDrop() const { return drop; }

    private:
        void face(bool side);
    };
}