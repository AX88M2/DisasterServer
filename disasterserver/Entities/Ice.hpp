#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class Ice : public Entity {
        uint8_t		iid = 0;
        bool		activated = false;
        double		timer = 0;
    public:
        Ice(entityId id, Server &server, GameState &state, const Vector2 &position, uint8_t iid);
        ~Ice() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        bool activate();

        uint8_t getIid() const { return iid; }
    };
}
