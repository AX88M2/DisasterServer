#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class SelfOp : public Command {
    public:
        SelfOp(Server &server, StateController &stateController);
        ~SelfOp() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}