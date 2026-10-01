#include "Debug.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Debuging::Debuging(Server &server, StateController &stateController) : Command(server, stateController, "debug") {
}

Debuging::~Debuging() = default;

void Debuging::execute(Client &client, CommandArguments &args) {
    if (!client.isOperator()) {
        this->server.sendMessage(client, "{}мяу сюка :3", CLRCODE_PUR);
        return;
    }

    CMD_GET_ARG(param_one, std::string, 0, "null")
    CMD_GET_ARG(param_two, std::string, 1, "null")

    this->server.sendMessage(client, "testing1: {}", param_one);
    this->server.sendMessage(client, "testing2: {}", param_two);
}
