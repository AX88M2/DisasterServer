#pragma once

#include "Characters/Character.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer::Characters
{
    class Tails : public Character {
        Countdown countdown;
    public:
        explicit Tails(Server &server, Client &client);
        ~Tails() override;

        void tick() override;
        void demonize() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}
