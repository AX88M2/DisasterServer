#include "CommandController.hpp"

#include <vector>
#include <regex>
#include <boost/algorithm/string.hpp>

#include "Commands/Ban.hpp"
#include "Commands/Kick.hpp"
#include "Commands/Op.hpp"
#include "Commands/Lobby.hpp"
#include "Commands/Help.hpp"
#include "Commands/Info.hpp"
#include "Commands/SelfOp.hpp"
#include "Commands/Map.hpp"
#include "Commands/Debug.hpp"
#include "Core/Log.hpp"

using namespace DisasterServer;

CommandController::CommandController(Server &server, StateController &stateController, MapController &mapController) :
    server(server), stateController(stateController), mapController(mapController)
{
    this->registerCommand<Commands::Ban>();
    this->registerCommand<Commands::Kick>();
    this->registerCommand<Commands::Op>();
    this->registerCommand<Commands::Lobby>();
    this->registerCommand<Commands::Help>();
    this->registerCommand<Commands::Information>();
    this->registerCommand<Commands::SelfOp>();
    this->registerCommand<Commands::Map>();
    this->registerCommand<Commands::Debuging>();
}

bool CommandController::process(Client &client, std::string &message) {
    try {
        auto executedCommand = this->parseCommand(message);

        const auto it = std::ranges::find_if(commands, [executedCommand](const auto& cmd) {
            return cmd->name == executedCommand.first;
        });

        if (it != commands.end()) {
            (*it)->execute(client, executedCommand.second);
            return true;
        }

        return false;
    } catch (NotFoundCommandPrefix &) {
        return false;
    }
}

std::pair<std::string, CommandArguments> CommandController::parseCommand(const std::string &str) {
    std::vector<std::string> arguments = {};

    if (str[0] != '.') {
        throw NotFoundCommandPrefix();
    }

    const std::regex regex(R"(^\.([^\s]+)(?:\s+(.+))?$)");
    std::smatch matches;
    std::regex_search(str, matches, regex);

    std::string cmd = matches[1].str();
    std::string cmdArgs = matches[2].str();

    try {
        if (!cmdArgs.empty()) {
            boost::split(arguments, cmdArgs, boost::is_any_of(" "));
        }
    } catch (std::exception const &e) {
        Error("Failed to parse arguments command {}: {}", cmd, e.what());
    }

    return { cmd, CommandArguments(std::move(arguments)) };
}
