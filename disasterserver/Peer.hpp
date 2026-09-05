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
        uint32_t id = 0;
        std::string ip = "";

        /* General info */
        //Player plr;
        std::string nickname;
        std::string udid;
        uint8_t lobby_icon = 0;
        int8_t pet = -1;

        bool verified = false;
        bool in_game = false;
        bool op = false;
        bool ready = false;
        bool mod_tool = false;
        bool is_mobile = false;
        bool can_vote = false;
        bool voted = false;

        struct AuthPeer {
            uint32_t type = 0;
            uint8_t	 one = 0;
            uint8_t	 two = 0;
        };

        AuthPeer auth = {};

        /* Character */
        SurvCharacters survChar = SurvCharacters::CH_NONE;
        ExesCharacters exeChar = ExesCharacters::EX_NONE;

        bool should_timeout = false;

        /* State info */
        uint8_t exe_chance = 0;
        double timeout = 0;
        double vote_cooldown = 0;

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

        void setExeChance(uint8_t chance) { exe_chance = chance; }
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
