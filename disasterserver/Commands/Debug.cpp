#include "Debug.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Debuging::Debuging(Server &server, StateController &stateController) : Command(server, stateController, "debug") {
}

Debuging::~Debuging() = default;

void Debuging::execute(Client &client, CommandArguments &args) {
    if (!client.isOperator()) {
        this->server.sendMessage(client, "{}иди нахуй (мяу :3)", CLRCODE_PUR);
        return;
    }

    this->server.sendMessage(client, "ты блять :)");

    this->server.sendMessage(client, "testing: {}", args.get<int>(0));
}
