#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class MJew : public Entity {
        enum class States : uint8_t {
            WAIT,
            PREPARE,
            FIRE
        };

        double nextTimer;
        double timer = 0;
        States state = States::WAIT;
    public:
        MJew(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~MJew() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}
