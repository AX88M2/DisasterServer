#pragma once

#include "BaseRing.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Ring : public BaseRing {
    public:
        Ring(entityId id, Server &server, GameState &state, const Vector2 &position, bool red = false);
        ~Ring() override;

        bool create() override;
        bool destroy() override;
    };
}