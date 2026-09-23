#pragma once
#include <cstdint>
#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {

    class Server;
    class GameState;

    class LCChain : public Entity {
    public:
        enum class State : uint8_t {
            NONE,
            PREPARE,
            ACTIVATE
        };

        LCChain(entityId id, Server &server, GameState &state, const Vector2 &pos);

        bool tick() override;

        double timer = 0.0;
        State state = State::NONE;
    };

}