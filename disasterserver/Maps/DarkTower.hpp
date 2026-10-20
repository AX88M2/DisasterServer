#pragma once

#include "Maps/Map.hpp"

namespace DisasterServer::Maps
{
    class DarkTower : public Map {
    public:
        explicit DarkTower(Server &server);

        void init(GameState &game) override;

        void tick() override;

        void handle(Client &client, Packet &packet) override;

        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}