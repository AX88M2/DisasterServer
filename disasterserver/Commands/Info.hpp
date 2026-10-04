#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Information : public Command {
    public:
        Information(Server &server, StateController &stateController, MapController &mapController);
        ~Information() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}