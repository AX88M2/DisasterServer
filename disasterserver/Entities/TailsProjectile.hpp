#pragma once

#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {
    class Server;
    class GameState;
}

namespace DisasterServer::Entities
{
    class TProjectile : public Entity {
        clientId owner = 0;
        int8_t dir = 0;
        uint8_t isExe = 0;
        uint8_t charge = 0;
        uint8_t damage = 0;

        double timer = 5.0 * TICKS_PER_SEC;
    public:
        TProjectile(entityId id, Server &server, GameState &state, const Vector2 &pos, clientId owner, int8_t dir, uint8_t exe, uint8_t charge, uint8_t damage);

        bool init() override;
        bool tick() override;
        bool uninit() override;
    };
}