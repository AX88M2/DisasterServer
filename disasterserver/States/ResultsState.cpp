#include "ResultsState.hpp"

#include "LobbyState.hpp"
#include "Server.hpp"

using namespace DisasterServer;

constexpr uint8_t PLRSTATE_ESCAPED = 4;
constexpr uint8_t PLRSTATE_ALIVE = 3;
constexpr uint8_t PLRSTATE_DEAD = 2;
constexpr uint8_t PLRSTATE_DEMONIZED = 1;
constexpr uint8_t PLRSTATE_EXE = 0;

ResultsState::ResultsState(Server &server, StateController &stateController,
        clientId exe,
        Ending ending,
        mapId id,
        uint16_t mapTimeSec,
        std::vector<std::unique_ptr<Client>> leftClients
    ) : State(server, stateController),
id(id), mapTimeSec(mapTimeSec), exe(exe), ending(ending), leftClients(std::move(leftClients)) {}

ResultsState::~ResultsState() = default;

void ResultsState::enter() {
    Debug("Attepting to enter DisasterServer::ResultsState...");

    countdown.start(15);

    Packet packet(PacketType::SERVER_RESULTS);
    packet.write(id);
    packet.sendBroadcast(server);

    Info("Server is now in {}Results", CLRCODE_PUR);
}

void ResultsState::exit() {
    for (auto &client : server.getClients()) {
        if (!client->isInGame()) {
            continue;
        }

        client->setSurvCharacter(SurvCharacters::NONE);
        client->setExeCharacter(ExesCharacters::NONE);
    }
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
            std::vector<Client> results;
            for (auto &c : server.getClients()) {
                if (!c->isInGame()) {
                    continue;
                }
                results.push_back(*c);
            }

            for (auto &c : leftClients) {
                if (!c->isInGame()) {
                    continue;
                }

                results.push_back(*c);
            }

            //TODO: Add sort

            for (auto c : results) {
                RAssert(sendResult(client, c, c.getPlayer().isFlag(Player::Flags::PLAYER_LEFT)));
            }

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

            Commands cmd = stateController.cmdParse(message);
            bool isCommand = stateController.cmdHandle(client, cmd, message);

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

bool ResultsState::sendResult(Client &client, Client &data, bool hasQuit) {
    uint8_t type = PLRSTATE_ALIVE;

    auto otherPlayer = data.getPlayer();

    if (otherPlayer.isFlag(Player::Flags::PLAYER_ESCAPED)) {
        type = PLRSTATE_ESCAPED;
    } else if (otherPlayer.isFlag(Player::Flags::PLAYER_DEMONIZED)) {
        type = PLRSTATE_DEMONIZED;
    } else if (otherPlayer.isFlag(Player::Flags::PLAYER_DEAD)) {
        type = PLRSTATE_DEAD;
    } else if (exe == data.getId()) {
        type = PLRSTATE_EXE;
    }

    Packet packet(PacketType::SERVER_RESULTS_DATA);

    packet.writeString(std::format("{}", data.getNickname()));
    packet.write<uint8_t>(data.getExeCharacter() != ExesCharacters::NONE ? static_cast<uint8_t>(data.getExeCharacter()) : static_cast<uint8_t>(data.getSurvCharacter()));
    packet.write<Ending>(ending);
    packet.write<uint16_t>(mapTimeSec);
    packet.write<uint8_t>(hasQuit);
    packet.write<uint8_t>(type);

    const auto status = otherPlayer.getStats();

    packet.write<uint16_t>(status.getRings());
    packet.write<uint16_t>(status.getKills());
    packet.write<uint16_t>(status.getDamage());
    packet.write<uint16_t>(status.getDamageTaken());
    packet.write<uint16_t>(status.getStunTime());
    packet.write<uint16_t>(status.getStuns());
    packet.write<uint16_t>(status.getHpRestored());
    packet.write<double>(status.getSurviveTime());
    packet.write<double>(status.getDangerTime());

    return packet.send(client, true);
}
