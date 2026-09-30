#include "SelfOp.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

SelfOp::SelfOp(Server &server, StateController &stateController) : Command(server, stateController, "selfop") {
}

SelfOp::~SelfOp() = default;

void SelfOp::execute(Client &client, CommandArguments &args) {
    if (client.getIp() != "127.0.0.1") {
        return;
    }

    server.getApplication().getStorage().addOperator(client);
    client.setOperator(true);
    server.sendMessage(client, "{}you're an operator now", CLRCODE_GRN);
}
