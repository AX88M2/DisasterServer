#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class Act9 : public Map {
    public:
        explicit Act9(MapContext context);

        void init() override;

        void tick() override;

        void handle(Client &client, Packet &packet) override;

        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}
