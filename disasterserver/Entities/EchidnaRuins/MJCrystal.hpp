#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class MJCrystal : public Entity {
        double nextTime = 10 * TICKS_PER_SEC;
        double timer = 0;
        bool state = false;
    public:
        MJCrystal(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~MJCrystal() override;

        bool create() override;
        bool tick() override;
        bool destroy() override;
    };
}
