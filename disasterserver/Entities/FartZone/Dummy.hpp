#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Dummy : public Entity {
        double vel = 0;
    public:
        Dummy(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~Dummy() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        void activate(int8_t dir);
    };
}
