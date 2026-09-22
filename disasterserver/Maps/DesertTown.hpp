#ifndef DISASTERSERVER_DESERTTOWN_HPP
#define DISASTERSERVER_DESERTTOWN_HPP

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class DesertTown : public Map
    {
    public:
        explicit DesertTown();

        void init(GameState& game) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };
}

#endif