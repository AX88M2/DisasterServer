#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Debuging : public Command {
    public:
        Debuging(Server &server, StateController &stateController);
        ~Debuging() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}