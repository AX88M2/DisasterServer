#include "StateController.hpp"
#include "Server.hpp"

#include "Core/Colors.hpp"

using namespace DisasterServer;

StateController::StateController(Server *server): server(server), lobby(server, this), charSelect(server, this) {
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

    switch (state) {
        case States::LOBBY:
        case States::MAPVOTE:
            return lobby.joined(peer);

        case States::CHARSELECT: {
            return charSelect.joined(peer);
        }
        case States::GAME: {
            return true;
        }
        case States::RESULTS: {
            break;
        }
        default: break;
    }

    return true;
}

void StateController::playerLeft(Client &peer) {
    Packet packet(PacketType::SERVER_PLAYER_LEFT);
    packet.write<clientId>(peer.getId());
    packet.sendBroadcast(*server, true);

    switch (state)
    {
        case States::LOBBY:
        case States::MAPVOTE:
            lobby.leaved(peer);
            break;

        case States::CHARSELECT:
            charSelect.leaved(peer);
            break;

        case States::GAME:
            //game_state_tick(server);
            break;

        case States::RESULTS:
            //results_state_tick(server);
            break;
    }
}

void StateController::tick() {
    switch (state)
    {
        case States::LOBBY:
        case States::MAPVOTE:
            lobby.tick();
            break;

        case States::CHARSELECT:
            charSelect.tick();
            break;

        case States::GAME:
            // game_state_tick(server);
            break;

        case States::RESULTS:
            // results_state_tick(server);
            break;
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
    switch (state) {
        case States::LOBBY:
        case States::MAPVOTE:
            lobby.handle(peer, packet);
            break;

        case States::CHARSELECT:
            charSelect.handle(peer, packet);
            break;

        case States::GAME:
            // game state handle
            break;

        case States::RESULTS:
            // results state handle
            break;
    }

    return true;
}

commandHash StateController::cmd_parse(std::string string) {
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

bool StateController::cmd_handle(Client &client, commandHash hash, const std::string &message) {
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
            RAssert(pack.send(client));
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
            RAssert(pack.send(client));
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
            RAssert(pack.send(client));
            break;
        }

        case CMD_LOBBY: {
            int ind;
            if (sscanf(message.c_str(), ".lobby %d", &ind) <= 0)
            {
                this->server->send_message(client, "{}example:~ .lobby 1");
                break;
            }
            break;
        }

        case CMD_HELP: {
            this->server->send_message(client, "~-----~ {}command list:{} ~-----~", CLRCODE_GRN, CLRCODE_RST);
            this->server->send_message(client, "|- .vk~ - vote kick ");
            this->server->send_message(client, "|- .info~ - information about server");
            this->server->send_message(client, "|- .vp~ - vote practice mode (wip)");
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
            this->server->send_message(client, "{}you aren't an operator.", CLRCODE_GRN);
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
