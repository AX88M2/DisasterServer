#pragma once
#include "Map.hpp"

namespace DisasterServer::Maps
{
    class NastyParadise : public Map {
    public:
        explicit NastyParadise(Server &server) : Map(server, "Nasty Paradise", 1, 30) {}

        void init(GameState&) override {}
        void tick() override {}
        void handle(Client&, Packet&) override {}
        void left(Client&) override {}
        MapProperties getMapProperties() const override {
            return MapProperties(3 * TICKS_PER_SEC, 10, 5);
        }
    };
}