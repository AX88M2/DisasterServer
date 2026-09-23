#pragma once
#include <cstdint>
#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {

    class Server;
    class GameState;

    class Latern : public Entity {
    public:
        Latern(entityId id, Server &server, GameState &state, const Vector2 &pos);

        bool tick() override;

        bool side  = false;
        double timer = 0.0;
        uint8_t lid   = 0;
        uint16_t time  = 0;
    };

}