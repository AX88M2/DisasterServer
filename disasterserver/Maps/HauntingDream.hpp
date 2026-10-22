#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class HauntingDream : public Map {
        GameState *gameCtx = nullptr;
    public:
        explicit HauntingDream(Server &server);

        void init(GameState &game) override;

        void tick() override;

        void handle(Client &client, Packet &packet) override;

        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}

