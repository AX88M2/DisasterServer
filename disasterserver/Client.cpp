#include "Client.hpp"

#include <utility>

#include "Server.hpp"
#include "Core/Log.hpp"
#include "Util/Packet.hpp"
#include "States/GameState.hpp"
#include "States/LobbyState.hpp"

using namespace DisasterServer;

Client::Client(Server *server, ENetPeer *peer, clientId incomingPeerID, std::string ip) :
    id(incomingPeerID),
    ip(std::move(ip)),
    peer(peer),
    server(server) {}

Client::~Client() = default;

bool Client::identity(Packet &packet) {
    RAssert(id > 0);

    if (packet.getPacketType() != PacketType::IDENTITY) {
        this->disconnect(DisconnectReason::OTHER, "type != IDENTITY?");
        return false;
    }

    bool isBanned = false;
    uint64_t timeout = 0;

    const uint16_t buildVersion = packet.read<uint16_t>();
    const int32_t serverIndex = packet.read<int32_t>();
    const std::string nickname = packet.readString();
    const std::string udid = packet.readString();
    const uint8_t lobbyIcon = packet.read<uint8_t>();
    const int8_t pet = packet.read<int8_t>();

    shouldTimeout = true;
    isModifiedClient = false;
    this->nickname = std::format("{}~", nickname);
    this->udid = udid;
    this->lobbyIcon = lobbyIcon;
    this->pet = pet;
    
    uint64_t rawKeyA = packet.read<uint64_t>();
    uint64_t rawKeyB = packet.read<uint64_t>();

    /* Verify auth */
    uint64_t keyA = rawKeyA - auth.type;
    uint64_t keyB = rawKeyB - auth.type;
    
    if ((auth.type >> 9) & 1) {
        if (keyA != 0x2f09cdda)
            isModifiedClient = true;

        if (keyB != 0xf1006056)
            isModifiedClient = true;
    }
    else if ((auth.type & 0x80000000) != 0) {
        if (keyA != 0x947)
            isModifiedClient = true;

        if (keyB != 0xb43)
            isModifiedClient = true;
    }
    else if ((auth.type >> 26) & 1) {
        if (keyA != 0xdcd)
            isModifiedClient = true;

        if (keyB != 0xc15)
            isModifiedClient = true;
    } else {
        isModifiedClient = true;
    }

    this->in_game = server->getStateController().isState<LobbyState>();
    this->exeChance = 1 + rand() % 4;

    if (this->server->getClients().size() >= MAX_PLAYERS) {
        this->disconnect(DisconnectReason::LOBBYFULL);
        return false;
    }

    if (buildVersion != BUILD_VERSION) {
        this->disconnect(DisconnectReason::VERMISMATCH);
        return false;
    }

    if (nickname.length() >= 30) {
        this->disconnect(DisconnectReason::OTHER, "Your nickname is too long! (30 characters max)");
        return false;
    }

    if (udid.empty()) {
        this->disconnect(DisconnectReason::OTHER, "whoops you have to put the CD in you conputer");
        return false;
    }

    if (!identityProcess(ip, isBanned, timeout, serverIndex == -1)) {
        return false;
    }

    Info("{} (id {}) joined.", this->nickname, id);
    Info("	IP: {}", ip);
    Info("	UID: {}", this->udid);
    Info("	Modified: {}", BoolStringify(isModifiedClient));

    this->verified = true;

    return true;
}

bool Client::identityProcess(const std::string &addr, bool is_banned, uint64_t timeout, bool do_timeout) {
    if (is_banned) {
        Info("{} banned by host (id {}, ip {})", nickname, id, addr);
        this->disconnect(DisconnectReason::BANNEDBYHOST);
        return false;
    }

    if (this->server->getClients().size() >= MAX_PLAYERS) {
        this->disconnect(DisconnectReason::LOBBYFULL);
        return false;
    }

    if (do_timeout && timeout != 0) {
        time_t tm = time(nullptr);
        time_t val = timeout - tm;
        if (val > 0) {
            Info("{} is rate-limited (id {}, ip {})", nickname, id, addr);
            this->disconnect(DisconnectReason::RATELIMITED);
            return false;
        }
    }

    if (!this->server->getStateController().playerJoined(*this)) {
        shouldTimeout = false;
        this->disconnect(DisconnectReason::OTHER, "Report this to dev: 415 baza otvette, mi tonem");
        return false;
    }

    Packet packet(PacketType::SERVER_IDENTITY_RESPONSE);
    packet.write<uint8_t>(server->getStateController().isState<LobbyState>());
    packet.write<clientId>(id);
    packet.send(*this, true);

    // If in queue, do following
    if (!in_game) {

        // For icons
        for (auto &client : this->server->getClients()) {
            if (client->getId() == id) {
                continue;
            }

            Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
            pack.write<uint8_t>(server->getStateController().isState<GameState>() && client->in_game);
            pack.write<clientId>(client->getId());
            pack.writeString(nickname);

            if (server->getStateController().isState<GameState>() && client->in_game) {

                pack.write<uint8_t>( 1 /* server->game.exe == peer->id */ );
                pack.write<uint8_t>( 1 /* server->game.exe == peer->id ? peer->exe_char : peer->surv_char */);

            } else {
                pack.write<uint8_t>(lobbyIcon);
            }

            pack.send(*this, true);
        }

        // For other players in queue
        Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
        pack.write<uint8_t>(0);
        pack.write<clientId>(id);
        pack.writeString(nickname);
        pack.write<uint8_t>(lobbyIcon);
        this->server->broadcastEx(pack, true, id);

        this->server->sendMessage(*this, "|build from &{} @{}~", __DATE__, __TIME__);
        this->server->sendMessage(*this, "|type .help for command list~");
    }

    return true;
}

bool Client::messageReceived(Packet &packet) {
    if (id == 0) {
        return false;
    }

    return this->server->getStateController().handle(*this, packet);
}

void Client::disconnect(DisconnectReason reason, const std::string &message) {
    if (disconnecting) {
        return;
    }

    if (reason == DisconnectReason::OTHER && !message.empty()) {
        Packet packet(PacketType::SERVER_PLAYER_FORCE_DISCONNECT);
        packet.write<DisconnectReason>(reason);
        packet.writeString(message);
        packet.send(*this, true);
        enet_peer_disconnect_later(peer, static_cast<uint32_t>(reason));
    } else {
        enet_peer_disconnect(peer, static_cast<uint32_t>(reason));
    }

    if (message.empty()) {
        Info("Disconnected id {} {}: No text.", id, getDisconnectReasonName(reason));
    } else {
        Info("Disconnected id {} {}: {}.", id, getDisconnectReasonName(reason), message);
    }

    disconnecting = true;
}
