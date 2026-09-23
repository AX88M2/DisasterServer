#ifndef DISASTERSERVER_YOUCANTRUN_HPP
#define DISASTERSERVER_YOUCANTRUN_HPP

#include "Map.hpp"

namespace DisasterServer::Maps {

    class YouCantRun : public Map {
    public:
        explicit YouCantRun();

        void init(GameState& game) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };

}

#endif