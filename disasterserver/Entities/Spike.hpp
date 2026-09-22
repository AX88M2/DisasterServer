#pragma once
#include <cstdint>
#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {

    class Server;
    class GameState;

    class SpikeController : public Entity {
        uint8_t frame = 0;
        double timer = 2.0 * TICKS_PER_SEC;
    public:
        SpikeController(entityId id, Server &server, GameState &state, const Vector2 &pos);
        ~SpikeController() override;
        bool tick() override;
    };

}