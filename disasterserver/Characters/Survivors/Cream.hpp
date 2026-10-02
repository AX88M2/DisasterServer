#pragma once

#include "Characters/Character.hpp"
#include "Core/Constansts.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer::Characters
{
    class Cream : public Character {
        Countdown countdown { TICKS_PER_SEC };
    public:
        explicit Cream(Server &server, Client &client);
        ~Cream() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}