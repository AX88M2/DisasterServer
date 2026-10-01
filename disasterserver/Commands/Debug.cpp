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

    auto arg1 = args.get<std::string>(0, "null");
    std::string a1 = arg1.has_value() ? arg1.value() : "null";

    this->server.sendMessage(client, "testing1: {}", a1);

    auto arg2 = args.get<std::string>(1, "null");
    std::string a2 = arg2.has_value() ? arg2.value() : "null";

    this->server.sendMessage(client, "testing2: {}", a2);
}
