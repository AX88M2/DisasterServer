#include "ResultsState.hpp"

#include "LobbyState.hpp"
#include "Server.hpp"

using namespace DisasterServer;

ResultsState::ResultsState(Server &server, StateController &stateController, mapId id) : State(server, stateController), id(id) {
}

void ResultsState::enter() {
    Debug("Attepting to enter DisasterServer::ResultsState...");

    countdown.start(15);

    Packet packet(PacketType::SERVER_RESULTS);
    packet.write(id);
    packet.sendBroadcast(server);

    Info("Server is now in {}Results", CLRCODE_PUR);
}

void ResultsState::exit() {
}

bool ResultsState::playerJoined(Client &client) {
    return State::playerJoined(client);
}

bool ResultsState::playerLeaved(Client &client) {
    return State::playerLeaved(client);
}

void ResultsState::tick() {
    auto result = countdown.tick(server.getDelta());
    switch (result) {
        case Countdown::TickResult::Finished: {
            stateController.changeTo<LobbyState>();
            break;
        }
        default: break;
    }
}

bool ResultsState::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_RESULTS_REQUEST: {

            break;
        }
        case PacketType::CLIENT_CHAT_MESSAGE: {
            if (client.isInGame()) break;

            [[maybe_unused]] const clientId pid = packet.read<clientId>();
            std::string message = packet.readString();

            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }

            client.setTimeout(0);

            commandHash hash = stateController.cmdParse(message);
            bool isCommand = stateController.cmdHandle(client, hash, message);

            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);

            if (!isCommand) {
                server.sendBroadcastMessage(client.getId(), message);
            }
            break;
        }
        default: break;
    }
    return true;
}
