#ifndef DISASTERSERVER_PEER_HPP
#define DISASTERSERVER_PEER_HPP

#include "Core/Packet.hpp"
#include "Core/ENet/server_listen_params.h"

namespace DisasterServer
{
    enum class SurvCharacters {
        CH_NONE = -1,

        CH_TAILS,
        CH_KNUX,
        CH_EGGMAN,
        CH_AMY,
        CH_CREAM,
        CH_SALLY
    };

    enum class ExesCharacters {
        EX_NONE = -1,

        EX_ORIGINAL,
        EX_CHAOS,
        EX_EXETIOR,
        EX_EXELLER
    };

    class Peer {
        uint16_t id;
        std::string ip;
        void *endpoint;

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

        /* Character */
        SurvCharacters survChars;
        ExesCharacters exesChars;

        bool should_timeout;

        /* State info */
        uint8_t exe_chance;
        double timeout;
        double vote_cooldown;

    public:
        Peer();
        ~Peer();

        static void initialize_client(Peer &client, const char *ip);

        uint32_t get_id();
        AuthPeer &getAuthPeer();


        bool identity(Packet &packet);
    };
}

#endif //DISASTERSERVER_PEER_HPP
