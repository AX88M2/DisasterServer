#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Shard : public Entity {
        uint8_t spawned;
    public:
        Shard(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t spawned);
        ~Shard() override;

        bool init() override;
    };
}