#ifndef DISASTERSERVER_LIMPCITY_HPP
#define DISASTERSERVER_LIMPCITY_HPP

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class LimpCity : public Map {
        GameState *game = nullptr;
    public:
        explicit LimpCity();

        void init(GameState& game) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };
}

#endif