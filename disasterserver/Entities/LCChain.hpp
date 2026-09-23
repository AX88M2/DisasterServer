#pragma once

#include "Entity.hpp"

namespace DisasterServer {
    class Server;
    class GameState;
}

namespace DisasterServer::Entities
{
    class LCChain : public Entity {
        enum class State : uint8_t {
            NONE,
            PREPARE,
            ACTIVATE
        };
        double timer = 0.0;
        State state = State::NONE;
    public:
        LCChain(entityId id, Server &server, GameState &state, const Vector2 &pos);

        bool tick() override;
    };
}