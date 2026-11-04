#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class PricelessFreedom : public Map {
    public:
        explicit PricelessFreedom(MapContext context);

        void init() override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapProperties() const override;
    };
}