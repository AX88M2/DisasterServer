#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Act9Wall : public Entity {
        uint8_t wid;
        double startTime = 0;
    public:
        Act9Wall(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t wid);
        ~Act9Wall() override;

        bool init() override;
        bool tick() override;
    };
}
