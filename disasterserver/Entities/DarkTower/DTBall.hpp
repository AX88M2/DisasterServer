#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class DTBall : public Entity {
        double state = 0;
        uint8_t side = 0;
    public:
        DTBall(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~DTBall() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}
