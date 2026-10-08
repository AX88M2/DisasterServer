#pragma once

#include "Map.hpp"

namespace DisasterServer::Maps
{
    class NotPerfect : public Map
    {
    public:
        explicit NotPerfect(Server &server);
        ~NotPerfect() noexcept override;

        void init(GameState& game) override;
        void tick() override;
        void handle(Client &client, Packet &packet) override;
        void left(Client &client) override;

        MapProperties getMapProperties() const override;
    };
}