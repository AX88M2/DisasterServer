#include "Info.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Information::Information(Server &server, StateController &stateController) : Command(server, stateController, "info") {
}

Information::~Information() = default;

void Information::execute(Client &client, CommandArguments &args) {
    server.sendMessage(client, "|build from &{} @{}~", __DATE__, __TIME__);
    server.sendMessage(client, "{}hander{} - original binary", CLRCODE_YLW, CLRCODE_RST);
    server.sendMessage(client, "{}miles{}glitch{} - rewritten server to c++", CLRCODE_BLU, CLRCODE_PUR, CLRCODE_RST);
    server.sendMessage(client, "{}faker{}null{}0{} - help with code", CLRCODE_GRA, CLRCODE_RED, CLRCODE_GRN, CLRCODE_RST);
}
