#include "Ban.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Ban::Ban(Server &server, StateController &stateController) : Command(server, stateController, "ban") {
}

Ban::~Ban() = default;

void Ban::execute(Client &client, CommandArguments &args) {
    if (!client.isOperator()) {
        server.sendMessage(client, "{}you aren't an operator.", CLRCODE_RED);
        return;
    }

    if (server.getInGameCount() <= 1) {
        server.sendMessage(client, "{}dude are you gonna ban yourself?", CLRCODE_RED);
        return;
    }

    Packet pack(PacketType::SERVER_LOBBY_CHOOSEBAN);
    if (!pack.send(client, true)) {
        Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client.getNickname(), client.getId());
    }
}
