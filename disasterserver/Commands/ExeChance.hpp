#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class ExeChance : public Command {
    public:
        ExeChance(Server &server, StateController &stateController, MapController &mapController);
        ~ExeChance() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}
