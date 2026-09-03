#include "Server.hpp"

#include <numbers>

#include "Core/Packet.hpp"

#include "Core/Log.hpp"
#include "Core/Time.hpp"

using namespace DisasterServer;

Server::Server() {

}

void Server::initialize() {
    server.start_listening(enetpp::server_listen_params<Peer>()
        .set_max_client_count(50)
        .set_channel_count(2)
        .set_listen_port(BASE_SERVER_PORT)
        .set_initialize_client_function(Peer::initialize_client));

    while (!running) {
        auto on_client_connected = [&](Peer& client) { this->on_client_connected(client); };
        auto on_client_disconnected = [&](unsigned int client_uid) { this->on_client_disconnected(client_uid); };
        auto on_client_received = [&](Peer& client, const enet_uint8* data, size_t data_size) { this->on_client_received(client, data, data_size); };

        server.consume_events(on_client_connected, on_client_disconnected, on_client_received);
    }

    server.stop_listening();
}

void Server::on_client_connected(Peer &peer) {
    Debug("Connection client {}", peer.get_id());

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

    preidentity.writeUint16(0);
    preidentity.writeUint16(1);
    preidentity.writeUint8(peer.getAuthPeer().one);
    preidentity.writeUint8((uint8_t) rand() % 2);
    preidentity.writeUint8(peer.getAuthPeer().two);

    const uint8_t key[6] = { 0x00, 0x00, 0xFF, 0x1F, 0x80, 0x14 };

    for (int i = 0; i < 3; i++) {
        preidentity.writeUint8(key[rand() % sizeof(key)]);
    }
    preidentity.writeUint32(peer.getAuthPeer().type);
    preidentity.send(server, peer.get_id(), true);

}

void Server::on_client_disconnected(uint32_t client_id) {

}

void Server::on_client_received(Peer &peer, const enet_uint8 *data, size_t data_size) {
    switch (static_cast<PacketType>(data[1])) {
        case PacketType::IDENTITY: {
            Packet packet(data, data_size);
            if (!peer.identity(packet)) {
                Debug("Identity failed for id {}", peer.get_id());
            }
            break;
        }
        default: break;
    }
}

/*
bool Server::worker() {

    TimeStamp ticker;
    time_start(&ticker);

    double next_tick = time_end(&ticker);
    double heartbeat = 0.0;
    const double TARGET_FPS = 1000.0 / 60;

    //Packet packet_heartbeat(PacketType::SERVER_HEARTBEAT);

    while (true) {
        udp::endpoint endpoint;
        socket.receive_from(boost::asio::buffer(recv_buffer), endpoint, 0, error_code);
        if (!error_code) {
            auto it = peers.find(endpoint);
            if (it == peers.end()) {
                Debug("Connection client {}:{}", endpoint.address().to_string(), endpoint.port());
                Peer peer(id_counter++, endpoint.address().to_string(), std::move(endpoint));
                peers.try_emplace(endpoint, std::move(peer));

                Debug("Creating peer id: {}", peer.getId());



            }

            switch (static_cast<PacketType>(recv_buffer[1])) {
                case PacketType::IDENTITY: {

                    Packet packet(recv_buffer);
                    Peer peer = peers.at(endpoint);

                    if (!peer.identity(packet)) {
                        Debug("Identity failed for id %d", peer.getId());
                    }

                    break;
                }
                default: {
                    break;
                }
            }
        }

        double now = time_end(&ticker);
        while (next_tick < now) {
            next_tick += TARGET_FPS;

            if (peers.empty()) {
                if (heartbeat >= (TICKSPERSEC * 2)) {
                    Debug("Heartbeat done.");
                    heartbeat = 0;
                }
            }

            delta = 1;
        }

    }
}
*/
