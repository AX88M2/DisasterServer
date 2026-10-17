#pragma once

#include <array>
#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class SnowBall : public Entity {
        uint8_t sid = 0;
        bool active = false;
        uint8_t state = 0;
        double stageProg = 0;
        int8_t dir;
        double frame = 0;
        double start = 0;
        double vel = 0;
        uint8_t pCount;
        double timer = 0;

        std::array<float, 20> pMove = {};
        std::array<float, 20> pAnim = {};
    public:
        SnowBall(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t sid, uint8_t pCount,
                int8_t dir, const std::array<float, 20> &pMove, const std::array<float, 20> &pAnim);
        ~SnowBall() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        void activate();
    };
}
