#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Chaos : public Character {
    public:
        explicit Chaos(Server &server, Client &client);
        ~Chaos() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}