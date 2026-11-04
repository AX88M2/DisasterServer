#pragma once

#include "Map.hpp"
#include "Util/Random.hpp"

namespace DisasterServer::Maps
{
    class RavineMist : public Map {
        Random random = {};
    public:
        explicit RavineMist(MapContext context);

        void init() override;
        void tick() override;
        void handle(Client &client, Packet &packet) override;
        void left(Client &client) override;

        MapProperties getMapProperties() const override;
        void checkOpeningBigring() override;
    private:

        void spawnShards(Client &client);
        void checkState();
    };
}
