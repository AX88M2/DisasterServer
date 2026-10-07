#include "Lobby.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Lobby::Lobby(Server &server, StateController &stateController, MapController &mapController) : Command(server, stateController, mapController, "lobby") {
}

Lobby::~Lobby() = default;

void Lobby::execute(Client &client, CommandArguments &args) {
    CMD_GET_ARG(ind, int, 0, -1)

    auto config = this->server.getApp().getConfigManager().config();

    if (ind < 1 || ind > config.getLobbyCount()) {
        this->server.sendMessage(client, "{}lobby should be between 1 and {}", CLRCODE_RED, config.getLobbyCount());
        return;
    }

    Packet pack(PacketType::SERVER_LOBBY_CHANGELOBBY);
    uint32_t port = config.getServerPort() + (ind - 1);
    pack.write<uint32_t>(port);

    if (!pack.send(client, true)) {
        Warn("Failed to send lobby change packet to {} (id {})", client.getNickname(), client.getId());
    }
}
