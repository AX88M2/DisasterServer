#ifndef DISASTERSERVER_PEER_HPP
#define DISASTERSERVER_PEER_HPP

#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "Core/Packet.hpp"

namespace DisasterServer
{
    class Peer : public boost::enable_shared_from_this<Peer> {
        uint16_t id;
        std::string ip;
        udp::endpoint endpoint;

        /* General info */
        //Player plr;
        std::string nickname;
        std::string udid;
        uint8_t lobby_icon;
        int8_t pet;

        bool verified;
        bool in_game;
        bool op;
        bool ready;
        bool mod_tool;
        bool is_mobile;
        bool can_vote;
        bool voted;
        bool disconnecting;

        struct AuthPeer {
            uint32_t type;
            uint8_t	 one;
            uint8_t	 two;
        };

        AuthPeer auth;

    public:
        Peer(uint16_t id, std::string ip, udp::endpoint endpoint);
        ~Peer();

        AuthPeer &getAuthPeer();
    };
}

#endif //DISASTERSERVER_PEER_HPP
