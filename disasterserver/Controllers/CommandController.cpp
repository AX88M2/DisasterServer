#include "CommandController.hpp"

#include <vector>

#include "Commands/Ban.hpp"
#include "Commands/Kick.hpp"
#include "Commands/Op.hpp"
#include "Commands/Lobby.hpp"
#include "Commands/Help.hpp"
#include "Commands/Info.hpp"
#include "Commands/SelfOp.hpp"
#include "Commands/Debug.hpp"
#include "Core/Log.hpp"

using namespace DisasterServer;

CommandController::CommandController(Server &server, StateController &stateController) : server(server), stateController(stateController) {
    this->registerCommand<Commands::Ban>();
    this->registerCommand<Commands::Kick>();
    this->registerCommand<Commands::Op>();
    this->registerCommand<Commands::Lobby>();
    this->registerCommand<Commands::Help>();
    this->registerCommand<Commands::Information>();
    this->registerCommand<Commands::SelfOp>();
    this->registerCommand<Commands::Debuging>();
}

bool CommandController::process(Client &client, std::string &message) {
    try {
        auto executedCommand = this->parseCommand(message);

        const auto it = std::ranges::find_if(commands, [executedCommand](const auto& cmd) {
            return cmd->name == executedCommand.first;
        });

        (*it)->execute(client, executedCommand.second);

        return true;
    } catch (NotFoundCommandPrefix &) {
        return false;
    }
}

std::pair<std::string, CommandArguments> CommandController::parseCommand(const std::string &str) {
    std::vector<std::string> arguments;
    std::string::size_type start = 0;

    const auto prefix = str.find_first_of('.', 0);

    if (prefix == std::string::npos) {
        throw NotFoundCommandPrefix();
    }

    auto cmd = str.substr(prefix + 1, str.size());

    auto end = cmd.find(' ', prefix);

    auto cmdName = cmd.substr(start, end - start);
    start = end + 1;
    end = cmd.find(' ', start);

    while (end == std::string::npos) {
        arguments.push_back(cmd.substr(start, end - start));

        start = end + 1;
        end = cmd.find(' ', start);
    }

    auto args = CommandArguments(arguments);

    return { cmdName, args };
}
