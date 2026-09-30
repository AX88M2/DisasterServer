#pragma once

#include <string>
#include <vector>

#include "Commands/Command.hpp"

namespace DisasterServer
{
    class Server;
    class StateController;

    class CommandController {
        Server& server;
        StateController &stateController;

        std::vector<std::unique_ptr<Command>> commands;
    public:
        CommandController(Server& server, StateController &stateController);

        bool process(Client& client, std::string &message);
    private:
        std::pair<std::string, CommandArguments> parseCommand(const std::string &str);

        template <std::derived_from<Command> T>
        void registerCommand() {
            auto next = std::make_unique<T>(server, stateController);
            commands.emplace_back(std::move(next));
        }
    };
}
