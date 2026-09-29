#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Cream : public Character {
    public:
        explicit Cream(Server &server, Client &client);
        ~Cream() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}