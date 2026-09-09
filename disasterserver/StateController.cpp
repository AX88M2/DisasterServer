#include "StateController.hpp"
#include "Server.hpp"
#include "Config.hpp"
#include "Core/Colors.hpp"

using namespace DisasterServer;

StateController::StateController(Server *server): server(server), current(std::make_unique<LobbyState>(server, this)) {
}

StateController::~StateController() = default;

bool StateController::playerJoined(Client &peer) {
    Packet packet(PacketType::SERVER_LOBBY_EXE_CHANCE);
    packet.write<uint8_t>(peer.getExeChance());
    packet.send(peer, true);

    Packet playerJoined(PacketType::SERVER_PLAYER_JOINED);
    playerJoined.write<clientId>(peer.getId());
    playerJoined.writeString(peer.getNickname());
    playerJoined.write<uint8_t>(peer.getLobbyIcon());
    playerJoined.write<uint8_t>(peer.getPet());
    this->server->broadcast_ex(playerJoined, true, peer.getId());

    if (current) {
        current->joined(peer);
    }

    return true;
}

void StateController::playerLeft(Client &peer) {
    Packet packet(PacketType::SERVER_PLAYER_LEFT);
    packet.write<clientId>(peer.getId());
    packet.sendBroadcast(*server, true);

    if (current) {
        current->leaved(peer);
    }
}

void StateController::tick() {
    if (current) {
        current->tick();
    }
}

bool StateController::handle(Client &peer, Packet &packet) {
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_LOBBY_CHOOSEBAN: {
            if (!peer.isOpped()) {
                break;
            }

            clientId pid = packet.read<clientId>();

            for (auto &c : server->getClients()) {
                if (c->getId() == pid) {
                    //TODO: add ban logic
                    c->disconnect(DisconnectReason::BANNEDBYHOST);
                }
            }

            break;
        }
        case PacketType::CLIENT_LOBBY_CHOOSEKICK: {
            if (!peer.isOpped()) {
                break;
            }

            clientId pid = packet.read<clientId>();

            for (auto &c : server->getClients()) {
                if (c->getId() == pid) {
                    //TODO: add kick logic
                    c->disconnect(DisconnectReason::KICKEDBYHOST);
                }
            }

            break;
        }
        case PacketType::CLIENT_LOBBY_CHOOSEOP: {
            if (!peer.isOpped()) {
                break;
            }

            clientId pid = packet.read<clientId>();

            for (auto &c : server->getClients()) {
                if (c->getId() == pid) {
                    //TODO: add operator logic
                    server->send_message(peer, "{}you're an operator now", CLRCODE_GRN);
                }
            }

            break;
        }
        default: break;
    }

    if (current) {
        current->handle(peer, packet);
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
            if (!client.isOpped()) {
                this->server->send_message(client, "{}you aren't an operator.", CLRCODE_RED);
                break;
            }

            if (this->server->getInGameCount() <= 1) {
                this->server->send_message(client, "{}dude are you gonna ban yourself?", CLRCODE_RED);
                break;
            }

            Packet pack(PacketType::CLIENT_LOBBY_CHOOSEBAN);
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getPacketType()), client.getNickname(), client.getId());
                return false;
            }
            break;
        }

        case CMD_KICK: {
            if (!client.isOpped()) {
                this->server->send_message(client, "{}you aren't an operator.", CLRCODE_RED);
                break;
            }

            if (this->server->getInGameCount() <= 1) {
                this->server->send_message(client, "{}dude are you gonna kick yourself?", CLRCODE_RED);
                break;
            }

            Packet pack(PacketType::SERVER_LOBBY_CHOOSEKICK);
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getPacketType()), client.getNickname(), client.getId());
                return false;
            }
            break;
        }

        case CMD_OP: {
            if (!client.isOpped()) {
                this->server->send_message(client, "{}you aren't an operator.", CLRCODE_RED);
                break;
            }

            if (this->server->getInGameCount() <= 1) {
                this->server->send_message(client, "{}you're already an operator tho??", CLRCODE_RED);
                break;
            }

            Packet pack(PacketType::SERVER_LOBBY_CHOOSEOP);
            if (!pack.send(client, true)) {
                Warn("Failed send packet {} to {} (id {})", getPacketTypeName(pack.getPacketType()), client.getNickname(), client.getId());
                return false;
            }
            break;
        }

        case CMD_LOBBY: {

            int ind;
            if (sscanf(message.c_str(), ".lobby %d", &ind) != 1) {
                this->server->send_message(client, "{}example: .lobby 1", CLRCODE_RED);
                break;
            }

            if (ind < 1 || ind > static_cast<int>(g_config.lobby_count)) {
                this->server->send_message(client, std::string(CLRCODE_RED) + "lobby should be between 1 and " + std::to_string(g_config.lobby_count));
                break;
            }

            Packet pack(PacketType::SERVER_LOBBY_CHANGELOBBY);
            uint32_t port = g_config.port + static_cast<uint32_t>(ind - 1);
            pack.write<uint32_t>(port);

            if (!pack.send(client, true)) {
                Warn("Failed to send lobby change packet to {} (id {})", client.getNickname(), client.getId());
            }

            break;
        }

        case CMD_HELP: {
            this->server->send_message(client, "~-----~ {}command list:{} ~-----~", CLRCODE_GRN, CLRCODE_RST);
            this->server->send_message(client, "|- .info~ - information about server");
            this->server->send_message(client, "|- .vk~ - vote kick");
            this->server->send_message(client, "|- .vp~ - vote practice mode (wip)");
            this->server->send_message(client, "|- .lobby~ - change lobby)");
            break;
        }

        case CMD_INFO: {
            this->server->send_message(client, "|build from &{} @{}~", __DATE__, __TIME__);
            this->server->send_message(client, "{}hander{} - original binary", CLRCODE_YLW, CLRCODE_RST);
            this->server->send_message(client, "{}miles{}glitch{} - rewritten server to c++", CLRCODE_BLU, CLRCODE_PUR, CLRCODE_RST);
            this->server->send_message(client, "{}faker{}null{}0{} - help with code", CLRCODE_GRA, CLRCODE_RED, CLRCODE_GRN, CLRCODE_RST);
            break;
        }

#if defined(SERVER_DEBUG)
        case CMD_SELFOP: {
            client.setOperator(true);
            this->server->send_message(client, "{}you're an operator now", CLRCODE_GRN);
            break;
        }
        case CMD_DEBUG: {
            if (!client.isOpped()) {
                this->server->send_message(client, "{}иди нахуй :3", CLRCODE_PUR);
                break;
            }
            break;
        }
#endif

        default: return false;
    }

    return true;
}
