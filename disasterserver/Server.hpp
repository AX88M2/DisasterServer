#ifndef DISASTERSERVER_SERVER_HPP
#define DISASTERSERVER_SERVER_HPP

#include "Peer.hpp"

#include "Core/ENet/server.h"

#define TICKSPERSEC 60

namespace DisasterServer
{
    static constexpr int MAX_PLAYERS = 7;
    static constexpr int BUILD_VERSION = 1101;
    static constexpr int BASE_SERVER_PORT = 8606;

    enum class DisconnectReason
    {
        FAILEDTOCONNECT,
        KICKEDBYHOST,
        BANNEDBYHOST,
        VERMISMATCH,
        SERVERTIMEOUT,
        PACKETSNOTRECV,
        GAMESTARTED,
        AFKTIMEOUT,
        LOBBYFULL,
        RATELIMITED,
        SHUTDOWN,
        IPINUSE,

        DONTREPORT = 254,
        OTHER = 255
    };

    inline std::string getDisconnectReasonName(DisconnectReason reason) {
        switch (reason) {
            case DisconnectReason::FAILEDTOCONNECT: return "FAILED TO CONNECT";
            case DisconnectReason::KICKEDBYHOST: return "KICKED BY HOST";
            case DisconnectReason::BANNEDBYHOST: return "BANNED BY HOST";
            case DisconnectReason::VERMISMATCH: return "VER MISMATCH";
            case DisconnectReason::SERVERTIMEOUT: return "SERVER TIMEOUT";
            case DisconnectReason::PACKETSNOTRECV: return "PACKET SNOT RECV";
            case DisconnectReason::GAMESTARTED: return "GAME STARTED";
            case DisconnectReason::AFKTIMEOUT: return "AFK TIMEOUT";
            case DisconnectReason::LOBBYFULL: return "LOBBY FULL";
            case DisconnectReason::RATELIMITED: return "RATE LIMITED";
            case DisconnectReason::SHUTDOWN: return "SHUTDOWN";
            case DisconnectReason::IPINUSE: return "IPINUSE";
            case DisconnectReason::DONTREPORT: return "DONT REPORT";
            case DisconnectReason::OTHER: return "OTHER";
            default: return "<Unknown>";
        }
    }

    class Server {
        bool running = false;
        enetpp::server<Peer> backend;
        double delta = 0;
    public:
        Server();
        void initialize();

        void disconnect(Peer &peer, DisconnectReason reason, const std::string& message = "");
        void disconnect_by_id(uint32_t id, DisconnectReason reason, const std::string& message = "");
        void broadcast(Packet &packet, bool reliable);
        void broadcast_ex(Packet &packet, bool reliable, uint32_t ignore);
        void send_message(uint32_t clientId, std::string &message);
        void send_message(uint32_t clientId, const char *message);
        void broadcast_message(uint16_t sender, std::string &message);
        std::vector<Peer*> getPeers() { return backend.get_connected_clients(); }

        enetpp::server<Peer>& getBackend() { return backend; }
    private:
        void on_client_connected(Peer &peer);
        void on_client_disconnected(Peer & peer, uint32_t client_id);
        void on_client_received(Peer &peer, const enet_uint8 *data, size_t data_size);

        bool state_handle(const Peer & peer, const Packet & packet);

        bool peer_message(Peer &peer, Packet &packet);
    };
}

#endif //DISASTERSERVER_SERVER_HPP
