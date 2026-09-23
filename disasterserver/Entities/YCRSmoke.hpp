#pragma once

#include "Entity.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {
    class Server;
    class GameState;
}

namespace DisasterServer::Entities
{
    class YCRController : public Entity {
        enum class State : uint8_t {
            NONE,
            SOME
        };

        State state = State::NONE;
        double timer = 0.0;
        uint8_t smokeId = 0;
        uint8_t activated = 0;
    public:
        YCRController(entityId id, Server &server, GameState &state, const Vector2 &pos);

        bool tick() override;
    };

}