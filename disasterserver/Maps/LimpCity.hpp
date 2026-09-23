#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class LimpCity : public Map {
        GameState *game = nullptr;
    public:
        explicit LimpCity(Server &server);

        void init(GameState& game) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapProperties() const override;
    };
}