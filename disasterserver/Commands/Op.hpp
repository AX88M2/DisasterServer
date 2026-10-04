#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Op : public Command {
    public:
        Op(Server &server, StateController &stateController, MapController &mapController);
        ~Op() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}