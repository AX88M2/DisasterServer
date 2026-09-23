#pragma once
#include <cstdint>
#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {

    class Server;
    class GameState;

    class LCEye : public Entity {
    public:
        LCEye(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t eyeId);

        bool tick() override;
        bool update();

        uint8_t  eyeId = 0;
        uint16_t useId = 0;
        bool used = false;
        uint8_t charge = 100;
        uint16_t target = 0;

        double cooldown = 0.0;
        double timer = 0.0;
    };

}