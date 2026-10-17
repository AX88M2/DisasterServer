#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Thunder : public Entity {
        double timer;
        bool flag = false;
    public:
        Thunder(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~Thunder() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}
