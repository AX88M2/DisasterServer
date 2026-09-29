#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class AmyRose : public Character {
    public:
        explicit AmyRose(Server &server, Client &client);
        ~AmyRose() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}