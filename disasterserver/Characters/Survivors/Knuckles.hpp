#pragma once

#include "Characters/Character.hpp"

namespace DisasterServer::Characters
{
    class Knuckles : public Character {
    public:
        static constexpr CharacterType Type = CharacterType::KNUCKLES;

        explicit Knuckles(Server &server, Client &client);
        ~Knuckles() override;

        void tick() override;
        bool handle(GameState& state, Packet &packet) override;
    };
}
