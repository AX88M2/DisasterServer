#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Exeller : public Character {
    public:
        explicit Exeller(Server &server, Client &client);
        ~Exeller() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}