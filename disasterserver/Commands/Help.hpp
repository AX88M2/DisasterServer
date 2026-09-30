#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Help : public Command {
    public:
        Help(Server &server, StateController &stateController);
        ~Help() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}