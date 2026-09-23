#pragma once
#include <cstdint>
#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {

    class Server;
    class GameState;

    class YCRController : public Entity {
    public:
        enum class State : uint8_t {
            NONE,
            SOME
        };

        YCRController(entityId id, Server &server, GameState &state, const Vector2 &pos);

        bool tick() override;

        State state = State::NONE;
        double timer = 0.0;
        uint8_t smokeId = 0;
        uint8_t activated = 0;
    };

}