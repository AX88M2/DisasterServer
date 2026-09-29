#pragma once

#include "Character.hpp"

namespace DisasterServer::Characters
{
    class UndefinedExe : public Character {
    public:
        UndefinedExe(Server &server, Client &client);
        ~UndefinedExe() override;
    };
}