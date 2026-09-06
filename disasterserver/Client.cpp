#include "Client.hpp"

#include "Server.hpp"
#include "Core/Log.hpp"
#include "Core/Packet.hpp"

using namespace DisasterServer;

Client::Client(Server *server, ENetPeer *peer, uint16_t incomingPeerID, const std::string &ip) :
    id(incomingPeerID),
    ip(ip),
    peer(peer),
    server(server) {}

Client::~Client() = default;

bool Client::identity(Packet &packet) {
    RAssert(id > 0);

    bool isBanned = false;
    uint64_t timeout = 0;

    uint16_t build_version = packet.read<uint16_t>();
    int32_t server_index = packet.read<int32_t>();
    std::string nickname = packet.readString();
    std::string udid = packet.readString();
    uint8_t lobby_icon = packet.read<uint8_t>();
    int8_t pet = packet.read<int8_t>();

    should_timeout = true;
    mod_tool = false;
    is_mobile = false;
    this->nickname = nickname;
    this->udid = udid;
    this->lobby_icon = lobby_icon;
    this->pet = pet;

    this->in_game = (server->getStateManager().getCurrentState() == States::LOBBY);
    this->exe_chance = 1 + rand() % 4;

    if (this->server->getPeers().size() >= MAX_PLAYERS) {
        this->disconnect(DisconnectReason::LOBBYFULL);
        return false;
    }

    if (packet.getPacketType() != PacketType::IDENTITY) {
        this->disconnect(DisconnectReason::OTHER, "type != IDENTITY?");
        return false;
    }

    if (build_version != BUILD_VERSION) {
        this->disconnect(DisconnectReason::VERMISMATCH);
        return false;
    }

    if (nickname.length() >= 30) {
        this->disconnect(DisconnectReason::OTHER, "Your nickname is too long! (30 characters max)");
        return false;
    }

    if (udid.length() <= 0) {
        this->disconnect(DisconnectReason::OTHER, "whoops you have to put the CD in you conputer");
        return false;
    }

    if (!identity_process(ip, isBanned, timeout, server_index == -1)) {
        return false;
    }

    Info("{} (id {}) joined.", nickname, id);
    Info("	IP: {}", ip);
    Info("	UID: {}", udid);
    Info("	Modified: {}", BoolStringify(mod_tool));
    Info("	Mobile: {}", BoolStringify(is_mobile));

    this->verified = true;

    return true;
}

bool Client::identity_process(const std::string &addr, bool is_banned, uint64_t timeout, bool do_timeout) {
    if (is_banned) {
        Info("{} banned by host (id {}, ip {})", nickname, id, addr);
        this->disconnect(DisconnectReason::BANNEDBYHOST);
        return false;
    }

    if (this->server->getPeers().size() >= 7) {
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

    if (!this->server->getStateManager().state_joined(*this)) {
        should_timeout = false;
        this->disconnect(DisconnectReason::OTHER, "Report this to dev: 415 baza otvette, mi tonem");
        return false;
    }

    Packet identityResponse(PacketType::SERVER_IDENTITY_RESPONSE);
    identityResponse.write<uint8_t>(server->getStateManager().getCurrentState() == States::LOBBY);
    identityResponse.write<uint16_t>(id);
    identityResponse.send(*this, true);

    // If in queue, do following
    if (!in_game) {

        // For icons
        for (auto &client : this->server->getPeers()) {
            if (client->getId() == id) {
                continue;
            }

            Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
            pack.write<uint8_t>(server->getStateManager().getCurrentState() == States::GAME && client->in_game);
            pack.write<uint16_t>(client->getId());
            pack.writeString(nickname);

            if (server->getStateManager().getCurrentState() == States::GAME && client->in_game) {

                pack.write<uint8_t>( 0 /* v->server->game.exe == peer->id */ );
                pack.write<uint8_t>( 0 /* v->server->game.exe == peer->id ? peer->exe_char : peer->surv_char */);

            } else {
                pack.write<uint8_t>(lobby_icon);
            }

            pack.send(*this, true);
        }

        // For other players in queue
        Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
        pack.write<uint8_t>(0);
        pack.write<uint16_t>(id);
        pack.writeString(nickname);
        pack.write<uint8_t>(lobby_icon);
        this->server->broadcast_ex(pack, true, id);

        this->server->send_message(*this, std::format("|build from &{} @{}~", __DATE__, __TIME__));
        this->server->send_message(*this, "|type .help for command list~");
    }

    return true;
}

bool Client::message_received(Packet &packet) {
    if (id == 0) {
        return false;
    }

    bool result;

    result = this->server->getStateManager().state_handle(*this, packet);

    return result;
}

void Client::disconnect(DisconnectReason reason, const std::string &message) {
    if (disconnecting) {
        return;
    }

    if (reason == DisconnectReason::OTHER && !message.empty()) {
        Packet packet(PacketType::SERVER_PLAYER_FORCE_DISCONNECT);
        packet.write<uint8_t>(static_cast<uint8_t>(reason));
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
