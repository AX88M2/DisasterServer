#pragma once

#include "Command.hpp"

namespace DisasterServer::Commands
{
    class Map : public Command {
    public:
        Map(Server &server, StateController &stateController, MapController &mapController);
        ~Map() override;

        void execute(Client &client, CommandArguments &args) override;
    };
}