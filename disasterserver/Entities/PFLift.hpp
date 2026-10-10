#pragma once

#include "Entity.hpp"

namespace DisasterServer::Entities
{
    class PFLift : public Entity {
        uint8_t lid = 0;
        double timer = 0.0;
        float start = 0.0f;
        float end = 0.0f;
        float speed = 0.0f;
        uint16_t activator = 0;
        bool activated = false;

    public:
        PFLift(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t lid, float start, float end);

        bool init() override;
        bool tick() override;
        bool activate(clientId pid);

        uint8_t getLid() const { return lid; }
    };
}