#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class FartZone : public Map {
        GameState *gameCtx = nullptr;
    public:
        explicit FartZone(MapContext context);

        void init() override;

        void tick() override;

        void handle(Client &client, Packet &packet) override;

        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}

