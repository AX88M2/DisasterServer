#include "Peer.hpp"

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

uint32_t Peer::get_id() {
    return id;
}

Peer::AuthPeer &Peer::getAuthPeer() {
    return auth;
}

bool Peer::identity(Packet &packet) {
    RAssert(id > 0);

    uint8_t passtrough = packet.read<uint8_t>();
    uint8_t type = packet.read<uint8_t>();
    uint16_t build_version = packet.read<uint16_t>();
    int32_t server_index = packet.read<int32_t>();
    std::string nickname = packet.readString();
    std::string udid = packet.readString();
    uint8_t lobby_icon = packet.read<uint8_t>();
    int8_t pet = packet.read<int8_t>();

    should_timeout = true;
    disconnecting = false;
    mod_tool = false;
    is_mobile = false;
    this->nickname = nickname;
    this->udid = udid;
    this->lobby_icon = lobby_icon;
    this->pet = pet;

    return true;
}
