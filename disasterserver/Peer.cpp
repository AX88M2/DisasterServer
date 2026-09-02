#include "Peer.hpp"

using namespace DisasterServer;

Peer::Peer(uint16_t id, std::string ip, udp::endpoint endpoint) : id(id), ip(ip), endpoint(endpoint) {

}

Peer::~Peer() = default;

Peer::AuthPeer &Peer::getAuthPeer() {
    return auth;
}