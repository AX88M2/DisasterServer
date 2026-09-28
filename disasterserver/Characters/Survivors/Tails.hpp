#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Tails : public Character {
    public:
        static constexpr CharacterType Type = CharacterType::TAILS;

        explicit Tails(Server &server, Client &client);
        ~Tails() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}