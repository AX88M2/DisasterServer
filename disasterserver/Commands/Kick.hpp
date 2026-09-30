#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Kick : public Command {
    public:
        Kick(Server &server, StateController &stateController);
        ~Kick() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}