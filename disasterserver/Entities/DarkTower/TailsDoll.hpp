#pragma once

#include "Entities/Entity.hpp"

namespace DisasterServer {
    class Server;
}

namespace DisasterServer::Entities
{
    class TailsDoll : public Entity {

        enum class States : uint8_t {
            NONE,
            READY,
            FOLLOW,
            RELOC
        };

        States state = States::NONE;
        uint32_t target = -1;
        double timer = 0;
        Vector2 vel = Vector2();
    public:
        TailsDoll(entityId id, Server &server, GameState &ctx, const Vector2 &position);
        ~TailsDoll() override;

        bool init() override;
        bool tick() override;
        bool uninit() override;

    private:
        void findSpot();
        bool isValidTarget();
        bool findTarget();
    };
}
