#ifndef DISASTERSERVER_KINDANDFAIR_HPP
#define DISASTERSERVER_KINDANDFAIR_HPP

#include "Map.hpp"

namespace DisasterServer::Maps {

    class KindAndFair : public Map {
    public:
        explicit KindAndFair();

        void init(GameState& game) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };

}

#endif