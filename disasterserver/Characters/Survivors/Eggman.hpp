#pragma once

#include "Characters/Character.hpp"
#include "Core/Constansts.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer::Characters
{
    class Eggman : public Character {
        Countdown countdown { TICKS_PER_SEC };
    public:
        explicit Eggman(Server &server, Client &client);
        ~Eggman() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}