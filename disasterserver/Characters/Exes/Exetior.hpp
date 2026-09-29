#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Exetior : public Character {
    public:
        explicit Exetior(Server &server, Client &client);
        ~Exetior() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}