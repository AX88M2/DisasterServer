#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class HDDoor : public Entity {
        uint8_t	state = 0;
        double timer = 0;
    public:
        HDDoor(entityId id, Server &server, GameState &state, const Vector2 &position);
        ~HDDoor() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

        void toggle();
    };
}
