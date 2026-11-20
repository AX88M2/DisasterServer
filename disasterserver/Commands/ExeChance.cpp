#include "ExeChance.hpp"

#include "Client.hpp"
#include "Server.hpp"
#include "States/LobbyState.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

ExeChance::ExeChance(Server &server, StateController &stateController, MapController &mapController) : Command(server, stateController, mapController, "exe") {
}

ExeChance::~ExeChance() = default;

void ExeChance::execute(Client &client, CommandArguments &args) {
    if (!stateController.isState<LobbyState>()) {
        server.sendMessage(client, "{}this command is only available in the lobby.", CLRCODE_RED);
        return;
    }

    if (!client.isOperator()) {
        return;
    }

    client.setExeChance(101);
    Packet pack(PacketType::SERVER_LOBBY_EXE_CHANCE);
    pack.write<uint8_t>(client.getExeChance());
    pack.send(client, true);
}


