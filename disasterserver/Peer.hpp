#ifndef DISASTERSERVER_PEER_HPP
#define DISASTERSERVER_PEER_HPP

#include "Core/Packet.hpp"

namespace DisasterServer
{
    class Server;

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
        uint32_t id;
        std::string ip;

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
        Server *server = nullptr;

        Peer();
        ~Peer();

        static void initialize_client(Peer &client, const char *ip);

        uint32_t getId() const { return id; }
        std::string getIp() const { return ip; }
        std::string getNickname() { return nickname; }
        std::string getUdid() { return udid; }
        uint8_t getLobbyIcon() { return lobby_icon; }
        int8_t getPet() { return pet; }
        AuthPeer &getAuthPeer() { return auth; }

        uint8_t getExeChance() { return exe_chance; }

        bool isVerified() { return verified; }
        bool isInGame() { return in_game; }
        bool isOpped() { return op; }
        bool isModified() { return mod_tool; }

        void setReady(bool flag) { ready = flag; }
        bool isReady() { return ready; }

        void setVoted(bool flag) { voted = flag; }
        bool isVoted() { return voted; }

        bool identity(Packet &packet);
        bool identity_process(const std::string & addr, bool is_banned, uint64_t timeout, bool do_timeout);
        bool message_received(Packet &packet);
    };
}

#endif //DISASTERSERVER_PEER_HPP
