#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include <boost/asio.hpp>

#include "Peer.hpp"
#include "Core/Packet.hpp"

#define TICKSPERSEC 60
#define BUILD_VERSION 1101
#define BASE_SERVER_PORT 8606

using boost::asio::ip::udp;

namespace DisasterServer
{
    class Server {
        boost::asio::io_context &io_context;
        udp::socket socket;
        std::array<uint8_t, PACKET_MAXSIZE> recv_buffer = {};

        uint16_t id_counter = 0;
        std::unordered_map<udp::endpoint, Peer> peers;
    public:
        Server(boost::asio::io_context& io_context);

    private:
        bool worker();
    };
}

#endif //DISASTERSERVER_SERVER_HPP
