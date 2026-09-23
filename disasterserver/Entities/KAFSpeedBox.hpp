#pragma once
#include <cstdint>
#include "Entity.hpp"
#include "Core/Constansts.hpp"
#include "Core/Vector2.hpp"

namespace DisasterServer {

    class Server;
    class GameState;

    class KafBox : public Entity {
    public:
        KafBox(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t nid);

        bool init() override;
        bool tick() override;
        bool activate(uint16_t pid, uint8_t isProj);

        uint8_t nid = 0;
        double timer = 0.0;
        bool activated = false;
    };

}