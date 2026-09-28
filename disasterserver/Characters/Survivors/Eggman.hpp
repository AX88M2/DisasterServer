#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Eggman : public Character {
    public:
        static constexpr CharacterType Type = CharacterType::EGGMAN;

        explicit Eggman(Server &server, Client &client);
        ~Eggman() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}