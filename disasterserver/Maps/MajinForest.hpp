#ifndef DISASTERSERVER_MAJINFOREST_HPP
#define DISASTERSERVER_MAJINFOREST_HPP

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class MajinForest : public Map
    {
    public:
        explicit MajinForest();

        void init(GameState& game) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };
}

#endif