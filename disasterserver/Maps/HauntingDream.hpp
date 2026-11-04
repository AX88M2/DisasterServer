#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class HauntingDream : public Map {
        GameState *gameCtx = nullptr;
    public:
        explicit HauntingDream(MapContext context);

        void init() override;

        void tick() override;

        void handle(Client &client, Packet &packet) override;

        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}

