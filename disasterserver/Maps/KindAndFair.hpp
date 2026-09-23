#ifndef DISASTERSERVER_KINDANDFAIR_HPP
#define DISASTERSERVER_KINDANDFAIR_HPP

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class KindAndFair : public Map {
        GameState *game = nullptr;
    public:
        explicit KindAndFair();

        void init(GameState& state) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };
}

#endif