#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Stalactits : public Entity {
        uint8_t	sid;
        bool state = false;
        bool show = false;
        Vector2 SPosition {};
        double timer = 0;
        float vel = 0;
    public:
        Stalactits(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t sid);
        ~Stalactits() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        bool activate();
    };
}
