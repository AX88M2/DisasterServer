#ifndef DISASTERSERVER_DOTDOTDOT_HPP
#define DISASTERSERVER_DOTDOTDOT_HPP

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class DotDotDot : public Map
    {
    public:
        explicit DotDotDot();

        void init(GameState& game) override;
        void tick() override;
        void handle(Client& client, Packet& packet) override;
        void left(Client& client) override;

        MapProperties getMapTime() const override;
    };
}

#endif