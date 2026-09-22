#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class BlackRing : public Entity {
    public:
        BlackRing(entityId id, Server &server, GameState &state);
        ~BlackRing() noexcept override;

        bool init() override;
        bool uninit() override;
    };
}
