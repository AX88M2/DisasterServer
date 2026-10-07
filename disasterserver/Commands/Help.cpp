#include "Help.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Help::Help(Server &server, StateController &stateController, MapController &mapController) : Command(server, stateController, mapController, "help") {
}

Help::~Help() = default;

void Help::execute(Client &client, CommandArguments &args) {
    server.sendMessage(client, "|- .info~ - information about server");
    server.sendMessage(client, "|- .vk~ - vote kick");
    server.sendMessage(client, "|- .vp~ - vote practice mode (wip)");
    server.sendMessage(client, "|- .lobby~ - change lobby (1-{})", this->server.getApp().getConfigManager().config().getLobbyCount());

    if(client.isOperator()) {
        server.sendMessage(client, "|- .map~ - force map (1-21)");
        server.sendMessage(client, "|- .kick~ - kick someone");
        server.sendMessage(client, "|- .ban~ - ban someone");
        server.sendMessage(client, "|- .op~ - op someone");
    }
}
