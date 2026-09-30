#include "Lobby.hpp"

#include "Server.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Commands;

Lobby::Lobby(Server &server, StateController &stateController) : Command(server, stateController, "lobby") {
}

Lobby::~Lobby() = default;

void Lobby::execute(Client &client, CommandArguments &args) {
    int ind;

    try {
        ind = args.get<int>(0);
    } catch (CommandException &) {
        this->server.sendMessage(client, "{}example: .lobby 1", CLRCODE_RED);
        return;
    }

    auto config = this->server.getApplication().getConfigManager().config();

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
