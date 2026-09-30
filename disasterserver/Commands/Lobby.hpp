#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Lobby : public Command {
    public:
        Lobby(Server &server, StateController &stateController);
        ~Lobby() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}