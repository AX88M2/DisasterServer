#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Vase : public Entity {
        uint8_t vid;
        uint8_t rings;
    public:
        Vase(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t vid);
        ~Vase() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        uint8_t getVid() { return vid; }
        uint8_t getRings() { return rings; }
    };
}
