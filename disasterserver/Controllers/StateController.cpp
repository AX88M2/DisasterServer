#include "StateController.hpp"
#include "Server.hpp"
#include "ConfigManager.hpp"
#include "Core/Constansts.hpp"
#include "States/LobbyState.hpp"
#include "States/GameState.hpp"

using namespace DisasterServer;

StateController::StateController(Server &server): server(server) {
    this->current = std::make_unique<LobbyState>(server, *this);
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
        default: break;
    }

    if (current) {
        return current->handle(client, packet);
    }

    return true;
}

commandHash StateController::cmdParse(std::string string) {
    static std::array clr_list = CLRLIST;

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

    return hash;
}

bool StateController::cmdHandle(Client &client, commandHash hash, const std::string &message) {
    switch (hash) {
        case CMD_BAN: {
            if (!client.isOperator()) {
                this->server.sendMessage(client, "{}you aren't an operator.", CLRCODE_RED);
                break;
            }

            if (this->server.getInGameCount() <= 1) {
                this->server.sendMessage(client, "{}dude are you gonna ban yourself?", CLRCODE_RED);
                break;
            }

            Packet pack(PacketType::SERVER_LOBBY_CHOOSEBAN);
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client.getNickname(), client.getId());
                return false;
            }
            break;
        }

        case CMD_KICK: {
            if (!client.isOperator()) {
                this->server.sendMessage(client, "{}you aren't an operator.", CLRCODE_RED);
                break;
            }

            if (this->server.getInGameCount() <= 1) {
                this->server.sendMessage(client, "{}dude are you gonna kick yourself?", CLRCODE_RED);
                break;
            }

            Packet pack(PacketType::SERVER_LOBBY_CHOOSEKICK);
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client.getNickname(), client.getId());
                return false;
            }
            break;
        }

        case CMD_OP: {
            if (!client.isOperator()) {
                this->server.sendMessage(client, "{}you aren't an operator.", CLRCODE_RED);
                break;
            }

            if (this->server.getInGameCount() <= 1) {
                this->server.sendMessage(client, "{}you're already an operator tho??", CLRCODE_RED);
                break;
            }

            Packet pack(PacketType::SERVER_LOBBY_CHOOSEOP);
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getType()), client.getNickname(), client.getId());
                return false;
            }
            break;
        }

        case CMD_LOBBY: {
            int ind;
            if (sscanf(message.c_str(), ".lobby %d", &ind) != 1) {
                this->server.sendMessage(client, "{}example: .lobby 1", CLRCODE_RED);
                break;
            }

            auto config = this->server.getApplication().getConfigManager().config();

            if (ind < 1 || ind > config.getLobbyCount()) {
                this->server.sendMessage(client, "{}lobby should be between 1 and {}", CLRCODE_RED, config.getLobbyCount());
                break;
            }

            Packet pack(PacketType::SERVER_LOBBY_CHANGELOBBY);
            uint32_t port = config.getServerPort() + (ind - 1);
            pack.write<uint32_t>(port);

            if (!pack.send(client, true)) {
                Warn("Failed to send lobby change packet to {} (id {})", client.getNickname(), client.getId());
            }

            break;
        }

        case CMD_HELP: {            
            this->server.sendMessage(client, "|- .info~ - information about server");
            this->server.sendMessage(client, "|- .vk~ - vote kick");
            this->server.sendMessage(client, "|- .vp~ - vote practice mode (wip)");
            this->server.sendMessage(client, "|- .lobby~ - change lobby (1-{})", this->server.getApplication().getConfigManager().config().getLobbyCount());

            if(client.isOperator())
            {
                this->server.sendMessage(client, "|- .map~ - force map (1-21)");
                this->server.sendMessage(client, "|- .kick~ - kick someone");
                this->server.sendMessage(client, "|- .ban~ - ban someone");
                this->server.sendMessage(client, "|- .op~ - op someone");
                break;
            }
            break;
        }

        case CMD_INFO: {
            this->server.sendMessage(client, "|build from &{} @{}~", __DATE__, __TIME__);
            this->server.sendMessage(client, "{}hander{} - original binary", CLRCODE_YLW, CLRCODE_RST);
            this->server.sendMessage(client, "{}miles{}glitch{} - rewritten server to c++", CLRCODE_BLU, CLRCODE_PUR, CLRCODE_RST);
            this->server.sendMessage(client, "{}faker{}null{}0{} - help with code", CLRCODE_GRA, CLRCODE_RED, CLRCODE_GRN, CLRCODE_RST);
            break;
        }

#if defined(SERVER_DEBUG)
        case CMD_SELFOP: {
            if (client.getIp() != "127.0.0.1") {
                break;
            }

            server.getApplication().getStorage().addOperator(client);
            client.setOperator(true);
            this->server.sendMessage(client, "{}you're an operator now", CLRCODE_GRN);
            break;
        }
        case CMD_DEBUG: {
            if (!client.isOperator()) {
                this->server.sendMessage(client, "{}иди нахуй (мяу :3)", CLRCODE_PUR);
                break;
            }

            break;
        }
#endif

        default: return false;
    }

    return true;
}