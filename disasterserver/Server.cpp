#include "Server.hpp"

#include <numbers>

#include "Core/Packet.hpp"
#include <boost/bind.hpp>

#include "Core/Log.hpp"

using namespace DisasterServer;

Server::Server(boost::asio::io_context &io_context) : io_context(io_context), socket(io_context, udp::endpoint(udp::v4(), BASE_SERVER_PORT)) {
    worker();
}

bool Server::worker() {
    boost::system::error_code error_code;
    socket.non_blocking(true, error_code);

    Packet packet(PacketType::SERVER_HEARTBEAT);

    for (;;) {
        udp::endpoint endpoint;
        socket.receive_from(boost::asio::buffer(recv_buffer), endpoint, 0, error_code);
        if (!error_code) {
            auto it = peers.find(endpoint);
            if (it == peers.end()) {
                Peer peer(id_counter++, endpoint.address().to_string(), std::move(endpoint));
                peers.try_emplace(endpoint, std::move(peer));

                Packet packet_preIdentity(PacketType::SERVER_PREIDENTITY);

                /* Auth client */
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

                packet_preIdentity.writeUint16(0);
                packet_preIdentity.writeUint16(1);
                packet_preIdentity.writeUint8(peer.getAuthPeer().one);
                packet_preIdentity.writeUint8((uint8_t) rand() % 2);
                packet_preIdentity.writeUint8(peer.getAuthPeer().two);

                const uint8_t key[6] = { 0x00, 0x00, 0xFF, 0x1F, 0x80, 0x14 };

                for (int i = 0; i < 3; i++) {
                    packet_preIdentity.writeUint8(key[rand() % sizeof(key)]);
                }

                packet_preIdentity.writeUint32(peer.getAuthPeer().type);

                packet_preIdentity.send(endpoint, socket);
            }
        }

        if (error_code != boost::asio::error::would_block && error_code != boost::asio::error::try_again) {
            return false;
        }
    }
}

