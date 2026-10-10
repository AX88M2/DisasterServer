#pragma once
#include "Map.hpp"

namespace DisasterServer::Maps
{
    class EntityController;

    class NastyParadise : public Map {
        GameState *gameCtx = nullptr;
    public:
        explicit NastyParadise(Server &server);

        void init(GameState&) override;
        void tick() override;
        void handle(Client&, Packet&) override;
        void left(Client&) override;
        MapProperties getMapProperties() const override;
    };
}