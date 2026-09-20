#ifndef DISASTERSERVER_HIDEANDSEEKACT2_HPP
#define DISASTERSERVER_HIDEANDSEEKACT2_HPP

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class HideAndSeekAct2 : public Map
    {
    public:
        explicit HideAndSeekAct2();

        void init() override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };
}

#endif