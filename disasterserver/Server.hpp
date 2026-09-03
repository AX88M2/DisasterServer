#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include "Peer.hpp"

#include "Core/ENet/server.h"

#define TICKSPERSEC 60
#define BUILD_VERSION 1101
#define BASE_SERVER_PORT 8606

namespace DisasterServer
{
    class Server {
        bool running = false;
        enetpp::server<Peer> server;
        double delta = 0;
    public:
        Server();
        void initialize();
    private:
        void on_client_connected(Peer &peer);
        void on_client_disconnected(uint32_t client_id);
        void on_client_received(Peer &peer, const enet_uint8 *data, size_t data_size);
        //bool worker();
    };
}

#endif //DISASTERSERVER_SERVER_HPP
