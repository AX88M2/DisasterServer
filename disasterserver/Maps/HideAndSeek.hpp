#ifndef DISASTERSERVER_HIDEANDSEEK_HPP
#define DISASTERSERVER_HIDEANDSEEK_HPP

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class HideAndSeek : public Map
    {
    public:
        explicit HideAndSeek();

        void init() override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };
}

#endif