#pragma once

#include "Character.hpp"

namespace DisasterServer::Characters
{
    class None : public Character {
    public:
        None(Server &server, Client &client);
        ~None() override;
    };
}

