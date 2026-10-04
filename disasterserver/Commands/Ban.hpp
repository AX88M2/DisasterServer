#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Ban : public Command {
    public:
        Ban(Server &server, StateController &stateController, MapController &mapController);
        ~Ban() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}