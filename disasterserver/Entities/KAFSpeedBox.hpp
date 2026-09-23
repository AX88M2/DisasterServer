#pragma once

#include "Entity.hpp"

namespace DisasterServer::Entities
{
    class KafBox : public Entity {
        uint8_t nid = 0;
        double timer = 0.0;
        bool activated = false;
    public:
        KafBox(entityId id, Server &server, GameState &state, const Vector2 &pos, uint8_t nid);

        bool init() override;
        bool tick() override;
        bool activate(clientId pid, uint8_t isProj);

        uint8_t getNid() const { return nid; };
    };

}