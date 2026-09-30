#include "Kick.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Kick::Kick(Server &server, StateController &stateController) : Command(server, stateController, "kick") {
}

Kick::~Kick() = default;

void Kick::execute(Client &client, CommandArguments &args) {
    if (!client.isOperator()) {
        server.sendMessage(client, "{}you aren't an operator.", CLRCODE_RED);
        return;
    }

    if (server.getInGameCount() <= 1) {
        server.sendMessage(client, "{}dude are you gonna kick yourself?", CLRCODE_RED);
        return;
    }

    Packet pack(PacketType::SERVER_LOBBY_CHOOSEKICK);
    if (!pack.send(client, true)) {
        Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client.getNickname(), client.getId());
    }
}
