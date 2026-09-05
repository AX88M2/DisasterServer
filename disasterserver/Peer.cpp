#include "Peer.hpp"

#include "Server.hpp"
#include "Core/Log.hpp"
#include "Core/Packet.hpp"

using namespace DisasterServer;

unsigned int next_uid = 1;

Peer::Peer() = default;

Peer::~Peer() = default;

void Peer::initialize_client(Peer &client, const char *ip) {
    client.id = next_uid++;
    client.ip = std::string(ip);
}

bool Peer::identity(Packet &packet) {
    RAssert(id > 0);

    bool isBanned = false;
    uint64_t timeout = 0;

    uint16_t build_version = packet.read<uint16_t>(); // NOLINT(*-use-auto)
    int32_t server_index = packet.read<int32_t>(); // NOLINT(*-use-auto)
    std::string nickname = packet.readString(); // NOLINT(*-use-auto)
    std::string udid = packet.readString(); // NOLINT(*-use-auto)
    uint8_t lobby_icon = packet.read<uint8_t>(); // NOLINT(*-use-auto)
    int8_t pet = packet.read<int8_t>(); // NOLINT(*-use-auto)

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
        this->server->disconnect(*this, DisconnectReason::LOBBYFULL);
        return false;
    }

    if (packet.getPacketType() != PacketType::IDENTITY) {
        this->server->disconnect(*this, DisconnectReason::OTHER, "type != IDENTITY?");
        return false;
    }

    if (build_version != BUILD_VERSION) {
        this->server->disconnect(*this, DisconnectReason::VERMISMATCH);
        return false;
    }

    if (nickname.length() >= 30) {
        this->server->disconnect(*this, DisconnectReason::OTHER, "Your nickname is too long! (30 characters max)");
        return false;
    }

    if (udid.length() <= 0) {
        this->server->disconnect(*this, DisconnectReason::OTHER, "whoops you have to put the CD in you conputer");
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

bool Peer::identity_process(const std::string &addr, bool is_banned, uint64_t timeout, bool do_timeout) {
    if (is_banned) {
        Info("{} banned by host (id {}, ip {})", nickname, id, addr);
        this->server->disconnect(*this, DisconnectReason::BANNEDBYHOST);
        return false;
    }

    if (this->server->getPeers().size() >= 7) {
        this->server->disconnect(*this, DisconnectReason::LOBBYFULL);
        return false;
    }

    if (do_timeout && timeout != 0) {
        time_t tm = time(nullptr);
        time_t val = timeout - tm;
        if (val > 0) {
            Info("{} is rate-limited (id {}, ip {})", nickname, id, addr);
            this->server->disconnect(*this, DisconnectReason::RATELIMITED);
            return false;
        }
    }

    if (!this->server->getStateManager().state_joined(*this)) {
        should_timeout = false;
        this->server->disconnect(*this, DisconnectReason::OTHER, "Report this to dev: 415 baza otvette, mi tonem");
        return false;
    }

    Packet identity_response(PacketType::SERVER_IDENTITY_RESPONSE);
    identity_response.write<uint8_t>(server->getStateManager().getCurrentState() == States::LOBBY);
    identity_response.write<uint16_t>(static_cast<uint16_t>(id));
    identity_response.send(*this->server, id, true);

    // If in queue, do following
    if (!in_game) {

        // For icons
        for (size_t i = 0; i < this->server->getPeers().size(); i++) {
            auto peer = this->server->getPeers()[i];

            if (peer->getId() == id) {
                continue;
            }

            Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
            pack.write<uint8_t>(server->getStateManager().getCurrentState() == States::GAME && peer->in_game);
            pack.write<uint16_t>(static_cast<uint16_t>(peer->getId()));
            pack.writeString(nickname);

            if (server->getStateManager().getCurrentState() == States::GAME && peer->in_game) {

                pack.write<uint8_t>( 0 /* v->server->game.exe == peer->id */ );
                pack.write<uint8_t>( 0 /* v->server->game.exe == peer->id ? peer->exe_char : peer->surv_char */);

            } else {
                pack.write<uint8_t>(lobby_icon);
            }

            pack.send(*this->server, id, true);
        }

        // For other players in queue
        Packet pack(PacketType::SERVER_WAITING_PLAYER_INFO);
        pack.write<uint8_t>(0);
        pack.write<uint16_t>(static_cast<uint16_t>(getId()));
        pack.writeString(nickname);
        pack.write<uint8_t>(lobby_icon);
        this->server->broadcast_ex(pack, true, id);

        this->server->send_message(id, "hello!");
    }

    return true;
}

bool Peer::message_received(Packet &packet) {
    if (id == 0) {
        return false;
    }

    bool result;

    result = this->server->getStateManager().state_handle(*this, packet);

    return result;
}
