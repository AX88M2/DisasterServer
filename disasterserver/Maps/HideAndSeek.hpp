#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class HideAndSeek : public Map
    {
    public:
        explicit HideAndSeek(MapContext context);

        void init() override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapProperties() const override;
    };
}