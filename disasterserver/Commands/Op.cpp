#include "Op.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Op::Op(Server &server, StateController &stateController) : Command(server, stateController, "op") {
}

Op::~Op() = default;

void Op::execute(Client &client, CommandArguments &args) {
    if (!client.isOperator()) {
        server.sendMessage(client, "{}you aren't an operator.", CLRCODE_RED);
        return;
    }

    if (server.getInGameCount() <= 1) {
        server.sendMessage(client, "{}you're already an operator tho??", CLRCODE_RED);
        return;
    }

    Packet pack(PacketType::SERVER_LOBBY_CHOOSEOP);
    if (!pack.send(client, true)) {
        Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client.getNickname(), client.getId());
    }
}
