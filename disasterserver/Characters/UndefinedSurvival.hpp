#pragma once

#include "Character.hpp"

namespace DisasterServer::Characters
{
    class UndefinedSurvival : public Character {
    public:
        UndefinedSurvival(Server &server, Client &client);
        ~UndefinedSurvival() override;
    };
}
