#include "StateController.hpp"
#include "Server.hpp"
#include "Configuration.hpp"
#include "Core/Constansts.hpp"
#include "States/LobbyState.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;

StateController::StateController(Server &server, MapController &mapController): server(server), mapController(mapController) {
    this->current = std::make_unique<LobbyState>(server, ContextControllers { *this, mapController });
}

StateController::~StateController() = default;

bool StateController::playerJoined(Client &client) {
    Packet packet(PacketType::SERVER_LOBBY_EXE_CHANCE);
    packet.write<uint8_t>(client.getExeChance());
    packet.send(client, true);

    Packet playerJoined(PacketType::SERVER_PLAYER_JOINED);
    playerJoined.write<clientId>(client.getId());
    playerJoined.writeString(client.getNickname());
    playerJoined.write<uint8_t>(client.getLobbyIcon());
    playerJoined.write<uint8_t>(client.getPet());
    this->server.broadcastEx(playerJoined, true, client.getId());

    if (current) {
        current->playerJoined(client);
    }

    return true;
}

void StateController::playerLeft(Client &client) {
    Packet packet(PacketType::SERVER_PLAYER_LEFT);
    packet.write<clientId>(client.getId());
    packet.sendBroadcast(server, true);

    if (current) {
        current->playerLeaved(client);
    }
}

void StateController::tick() {
    if (!pendingState.empty()) {
        for (auto &state : pendingState) {
            if (current) {
                current->exit();
            }
            current = std::move(state);
        }

        pendingState.clear();

        current->enter();
    }

    if (current) {
        current->tick();
    }
}

bool StateController::handle(Client &client, Packet &packet) {
    switch (packet.getType()) {
        case PacketType::CLIENT_LOBBY_CHOOSEBAN: {
            if (!client.isOperator()) {
                break;
            }

            clientId pid = packet.read<clientId>();

            for (auto &c : server.getClients()) {
                if (c->getId() == pid) {
                    server.getApplication().getStorage().addBan(*c);
                    c->disconnect(DisconnectReason::BANNEDBYHOST);
                }
            }

            break;
        }

        case PacketType::CLIENT_LOBBY_CHOOSEKICK: {
            if (!client.isOperator()) {
                break;
            }

            clientId pid = packet.read<clientId>();

            for (auto &c : server.getClients()) {
                if (c->getId() == pid) {
                    //TODO: add timeout logic
                    c->disconnect(DisconnectReason::KICKEDBYHOST);
                }
            }

            break;
        }

        case PacketType::CLIENT_LOBBY_CHOOSEOP: {
            if (!client.isOperator()) {
                break;
            }

            clientId pid = packet.read<clientId>();

            for (auto &c : server.getClients()) {
                if (c->getId() == pid) {
                    server.getApplication().getStorage().addOperator(*c);
                    c->setOperator(true);
                    server.sendMessage(client, "{}you're an operator now", CLRCODE_GRN);
                }
            }

            break;
        }

        case PacketType::CLIENT_CHAT_MESSAGE: {
            if (client.isInGame()) break;

            [[maybe_unused]] const clientId pid = packet.read<clientId>();
            std::string message = packet.readString();

            handleChat(client, message, [&](CommandsHash cmd, std::string &msg) {
                return cmdHandle(client, cmd, msg);
            });

            break;
        }

        default: break;
    }

    if (current) {
        return current->handle(client, packet);
    }

    return true;
}

CommandsHash StateController::cmdParse(std::string string) {
    std::string current;
    bool started = false;

    for (char ch : string)
    {
        if (!started && std::isspace(static_cast<unsigned char>(ch)))
            continue;

        started = true;

        if (std::isspace(static_cast<unsigned char>(ch)))
            break;

        bool invalid = false;

        for (int j = 0; j < CLRLIST_LEN; ++j)
        {
            if (ch == clr_list[j][0])
            {
                invalid = true;
                break;
            }
        }

        if (!invalid)
            current += ch;
    }

    unsigned long hash = 0;

    for (unsigned char ch : current)
        hash = 31 * hash + ch;

    return static_cast<CommandsHash>(hash);
}

void StateController::handleChat(Client &client, std::string &message, std::function<bool(CommandsHash, std::string &)> cmdProcessor) {
    client.setTimeout(0);

    if (message.length() > 90) {
        this->server.sendMessage(client, "{}Chat message too long", CLRCODE_RED);
        return;
    }

    CommandsHash cmd = cmdParse(message);
    bool isCommand = cmdProcessor(cmd, message);

    Info("{} (id {}): {}", client.getNickname(), client.getId(), message);

    if (!isCommand) {
        server.sendBroadcastMessage(client.getId(), message);
    }
}

bool StateController::cmdHandle(Client &client, CommandsHash hash, const std::string &msg) {
    std::string message(msg);
    return server.getCommandController().process(client, message);
}
