#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Original : public Character {
    public:
        explicit Original(Server &server, Client &client);
        ~Original() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}