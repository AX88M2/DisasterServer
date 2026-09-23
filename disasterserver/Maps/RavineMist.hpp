#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class RavineMist : public Map {
        GameState* state = nullptr;
    public:
        explicit RavineMist(Server &server);

        void init(GameState &game) override;
        void tick() override;
        void handle(Client &client, Packet &packet) override;
        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}