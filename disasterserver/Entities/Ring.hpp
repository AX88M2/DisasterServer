#pragma once
#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Ring : public Entity {
        uint8_t rid = 0;
        bool red;
    public:
        Ring(entityId id, Server &server, GameState &state, const Vector2 &position, bool red = false);
        ~Ring() override;

        bool init() override;
        bool uninit() override;

        bool isRed() const { return red; }
    };
}