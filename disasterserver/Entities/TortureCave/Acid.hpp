#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Acid : public Entity {
        uint8_t	acid_id = 0;
        uint8_t	activated = 0;
        double timer = 0.0;
    public:
        Acid(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~Acid() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}
