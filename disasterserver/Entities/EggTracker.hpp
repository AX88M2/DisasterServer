#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class EggTracker : public Entity {
        clientId activId = 0;
    public:
        EggTracker(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~EggTracker() override;

        bool init() override;
        bool uninit() override;

        void setActivId(const clientId id) { activId = id; };
    };
}