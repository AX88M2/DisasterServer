#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class NotPerfect : public Map
    {
    public:
        explicit NotPerfect(MapContext context);
        ~NotPerfect() noexcept override;

        void init() override;
        void tick() override;
        void handle(Client &client, Packet &packet) override;
        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}