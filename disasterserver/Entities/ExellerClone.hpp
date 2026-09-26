#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class ExellerClone : public Entity {
        int8_t dir;
        clientId owner;
    public:
        ExellerClone(entityId id, Server &server, GameState &state, const Vector2 &position, int8_t dir, clientId owner);
        ~ExellerClone() override;

        bool init() override;
        bool uninit() override;

        clientId getOwner() const { return owner; }
        int8_t getDir() const { return dir; }
    };
}
