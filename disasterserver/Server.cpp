#include "Server.hpp"

#include "Core/Packet.hpp"
#include "Core/Log.hpp"
#include "Core/Time.hpp"

using namespace DisasterServer;

Server::Server() = default;

void Server::initialize() {
    backend.start_listening(enetpp::server_listen_params<Peer>()
        .set_max_client_count(50)
        .set_channel_count(2)
        .set_listen_port(BASE_SERVER_PORT)
        .set_initialize_client_function(Peer::initialize_client));

    TimeStamp ticker;
    time_start(&ticker);

    double next_tick = time_end(&ticker);
    double heartbeat = 0.0;
    const double TARGET_FPS = 1000.0 / 60;



    while (!running) {
        backend.consume_events(
            [&](Peer& client) { this->on_client_connected(client); },
            [&](Peer &client, uint32_t client_uid) { this->on_client_disconnected(client, client_uid); },
            [&](Peer& client, const enet_uint8* data, size_t data_size) { this->on_client_received(client, data, data_size); }
        );
        double now = time_end(&ticker);
        while (next_tick < now) {
            next_tick += TARGET_FPS;

            //States tick


            // Heartbeat
            if (backend.get_connected_clients().empty()) {
                if (heartbeat >= (TICKSPERSEC * 2))
                {
                    Packet packet_heartbeat(PacketType::SERVER_HEARTBEAT);
                    packet_heartbeat.sendBroadcast(*this, true);
                    heartbeat = 0;
                }
                heartbeat += delta;
            }

            delta = 1;
        }
    }

    backend.stop_listening();
}

void Server::disconnect(Peer &peer, DisconnectReason reason, const std::string& message) {
    if (reason == DisconnectReason::OTHER && !message.empty()) {
        Packet packet(PacketType::SERVER_PLAYER_FORCE_DISCONNECT);
        packet.write<uint8_t>(static_cast<uint8_t>(reason));
        packet.writeString(message);
        packet.send(*this, peer.getId(), true);
        backend.disconnect_leter(peer.getId(), static_cast<uint32_t>(reason));
    } else {
        backend.disconnect(peer.getId(), static_cast<uint32_t>(reason));
    }

    if (message.empty()) {
        Info("Disconnected id {} {}: No text.", peer.getId(), getDisconnectReasonName(reason));
    } else {
        Info("Disconnected id {} {}: {}.", peer.getId(), getDisconnectReasonName(reason), message);
    }
}

void Server::disconnect_by_id(uint32_t id, DisconnectReason reason, const std::string &message) {
    for (size_t i = 0; i < backend.get_connected_clients().size(); i++) {
        auto client = backend.get_connected_clients()[i];
        if (client->getId() == id) {
            return disconnect(*client, reason, message);
        }
    }
}

void Server::broadcast(Packet &packet, bool reliable) {
    Debug("PacketType::{} sending broadcast", getPacketTypeName(packet.getPacketType()));
    packet.sendBroadcast(*this, reliable);
}

void Server::broadcast_ex(Packet &packet, bool reliable, uint32_t ignore) {
    Debug("PacketType::{} sending broadcast, ignoring client {}", getPacketTypeName(packet.getPacketType()), ignore);
    packet.sendBroadcast(*this, reliable, [&](const Peer& peer) {
        if (peer.getId() == ignore) {
            return false;
        }
        return true;
    });
}

void Server::send_message(uint32_t clientId, std::string &message) {
    Packet packet(PacketType::CLIENT_CHAT_MESSAGE);
    packet.write<uint16_t>(0);
    packet.writeString(message);
    packet.send(*this, clientId, true);
}

void Server::send_message(uint32_t clientId, const char *message) {
    Packet packet(PacketType::CLIENT_CHAT_MESSAGE);
    packet.write<uint16_t>(0);
    packet.writeString(message);
    packet.send(*this, clientId, true);
}

void Server::send_broadcast_message(uint16_t sender, std::string &message) {
    Packet packet(PacketType::CLIENT_CHAT_MESSAGE);

    packet.write<uint16_t>(sender);
    packet.writeString(message);

    broadcast(packet, true);
}

bool Server::state_joined(Peer &peer) {
    Packet packet(PacketType::SERVER_LOBBY_EXE_CHANCE);
    packet.write<uint8_t>(peer.getExeChance());
    packet.send(*this, peer.getId(), true);

    Packet playerJoined(PacketType::SERVER_PLAYER_JOINED);
    playerJoined.write<uint16_t>(static_cast<uint16_t>(peer.getId()));
    playerJoined.writeString(peer.getNickname());
    playerJoined.write<uint8_t>(peer.getLobbyIcon());
    playerJoined.write<uint8_t>(peer.getPet());
    this->broadcast_ex(playerJoined, true, peer.getId());

    return true;
}

bool Server::state_handle(Peer &peer, Packet &packet) {
    return true;
}

void Server::on_client_connected(Peer &peer) {
    peer.server = std::move(this);
    Debug("Connection client {}", peer.getId());

    Packet preidentity(PacketType::SERVER_PREIDENTITY);
    peer.getAuthPeer().one = (uint8_t) rand() % 128;
    peer.getAuthPeer().two = (uint8_t) rand() % 255;

    uint32_t type = 0;

    switch (rand() % 3)
    {
        case 0:
            type = 1u << 9;
            break;

        case 1:
            type = 1u << 31;
            break;

        case 2:
            type = 1u << 26;
            break;

        default: break;
    }

    for (int bit = 0; bit < 32; ++bit)
    {
        if (bit == 9 || bit == 31 || bit == 26)
            continue;

        if (rand() % 3 <= 1)
            type &= ~(1u << bit);
        else
            type |= 1u << bit;
    }

    peer.getAuthPeer().type = type;

    preidentity.write<uint16_t>(0);
    preidentity.write<uint16_t>(1);
    preidentity.write<uint8_t>(peer.getAuthPeer().one);
    preidentity.write<uint8_t>((uint8_t) rand() % 2);
    preidentity.write<uint8_t>(peer.getAuthPeer().two);

    const uint8_t key[6] = { 0x00, 0x00, 0xFF, 0x1F, 0x80, 0x14 };

    for (int i = 0; i < 3; i++) {
        preidentity.write<uint8_t>(key[rand() % sizeof(key)]);
    }
    preidentity.write<uint32_t>(peer.getAuthPeer().type);
    preidentity.send(*this, peer.getId(), true);

}

void Server::on_client_disconnected(Peer &peer, uint32_t client_id) {
    Info("{} (id {}) left.", peer.getNickname(), client_id);
}

void Server::on_client_received(Peer &peer, const enet_uint8 *data, size_t data_size) {
    Packet packet(data, data_size);

    switch (packet.getPacketType()) {
        case PacketType::IDENTITY: {
            if (!peer.identity(packet)) {
                Debug("Identity failed for id {}", peer.getId());
            }
            break;
        }
        default: {
            if (!peer.message_received(packet)) {
                break;
            }
        }
    }
}