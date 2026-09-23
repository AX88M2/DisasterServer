#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class KindAndFair : public Map {
        GameState *game = nullptr;
    public:
        explicit KindAndFair(Server &server);

        void init(GameState& state) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapProperties() const override;
    };
}