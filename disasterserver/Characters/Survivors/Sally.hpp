#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Sally : public Character {
    public:
        explicit Sally(Server &server, Client &client);
        ~Sally() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}