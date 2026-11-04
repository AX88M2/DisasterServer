#pragma once
#include "Map.hpp"

namespace DisasterServer::Maps
{
    class EntityController;

    class NastyParadise : public Map {
    public:
        explicit NastyParadise(MapContext context);

        void init() override;
        void tick() override;
        void handle(Client&, Packet&) override;
        void left(Client&) override;
        MapProperties getMapProperties() const override;
    };
}