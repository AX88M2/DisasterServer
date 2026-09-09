#include "MapVoteState.hpp"

#include <algorithm>

#include "StateController.hpp"
#include "Server.hpp"

using namespace DisasterServer;

MapVoteState::MapVoteState(Server *server, StateController *controller) : State(server, controller), countdown(server) {
}

void MapVoteState::init() {
    Debug("Attepting to enter ST_MAPVOTE...");

    // randomize
    time_t seed = time(nullptr);
    Debug("Mapvote seed: %d", seed);
    srand(static_cast<unsigned int>(seed));

    countdown.start(30);
    countdown.setEndOfCountdown([&] {

    });
}

bool MapVoteState::joined(Client &client) {
    return true;
}

bool MapVoteState::leaved(Client &client) {
    if (this->server->getInGameCount() <= 1) {
        controller->changeTo<LobbyState>();
        return true;
    }
    return true;
}

void MapVoteState::tick() {
    countdown.update();
}

bool MapVoteState::handle(Client &client, Packet &packet) {
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_VOTE_REQUEST: {
            if (client.isInGame()) {
                break;
            }

            uint8_t map = packet.read<uint8_t>();

            AssertOrDisconnect(client, !client.isVoted());
            AssertOrDisconnect(client, map > 0);
            AssertOrDisconnect(client, map < 3);

            votes[map]++;
            client.setVoted(true);

            Packet pack(PacketType::SERVER_VOTE_SET);
            for (int i = 0; i < 3; i++) {
                pack.write<uint8_t>(votes[i]);
            }

            pack.sendBroadcast(*server);
            checkState();
            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            const clientId pid = packet.read<clientId>();
            const std::string message = packet.readString();

            if (message.size() > 40) {
                client.disconnect(DisconnectReason::OTHER, "Chat message too long");
                return false;
            }

            client.setTimeout(0);

            commandHash hash = controller->cmdParse(message);
            bool isCommand = controller->cmdHandle(client, hash, message);

            Info("{} (id {}): {}", client.getNickname(), client.getId(), message);
            if (!isCommand) {
                server->send_broadcast_message(client.getId(), message);
            }

            break;
        }

        default: break;
    }

    return true;
}

void MapVoteState::checkState() {
    const auto players = &server->getClients();
    const auto count = std::ranges::count_if(
        *players,
        [](const auto& peer) {
            return peer->isInGame() && peer->isVoted();
        }
    );

    if (count <= this->server->getInGameCount()) {
        /*if (server->lobby.countdown_sec > 3)
        {
            server->lobby.countdown = 0;
            server->lobby.countdown_sec = 4;
        }*/
    }
}
