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

    class Client {
        uint16_t id;
        std::string ip;
        ENetPeer *peer;

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
        bool disconnecting = false;

        struct AuthPeer {
            uint32_t type = 0;
            uint8_t	 one = 0;
            uint8_t	 two = 0;
        } auth = {};

        /* Character */
        SurvCharacters survChar = SurvCharacters::CH_NONE;
        ExesCharacters exeChar = ExesCharacters::EX_NONE;

        bool should_timeout = false;

        /* State info */
        uint8_t exe_chance = 0;
        double timeout = 0;
        double vote_cooldown = 0;

        Server *server = nullptr;
    public:
        Client(Server *server, ENetPeer *peer, uint16_t incomingPeerID, const std::string &ip);
        ~Client();

        uint16_t getId() const { return id; }
        std::string getIp() const { return ip; }
        ENetPeer * getPeer() { return peer; }
        std::string getNickname() { return nickname; }
        std::string getUdid() { return udid; }
        uint8_t getLobbyIcon() { return lobby_icon; }
        int8_t getPet() { return pet; }
        AuthPeer &getAuthPeer() { return auth; }
        void setExeChance(uint8_t chance) { exe_chance = chance; }
        uint8_t getExeChance() { return exe_chance; }
        void setTimeout(double value) { timeout = value; }
        double getTimeout() const { return timeout; }
        bool isShouldTimeout() { return should_timeout; }
        bool isDisconnecting() { return disconnecting; }

        void setVoteCooldown(double value) { vote_cooldown = value; }
        double getVoteCooldown() { return vote_cooldown; }
        void setInGame(bool flag) { in_game = flag; }
        bool isInGame() { return in_game; }
        bool isVerified() { return verified; }
        bool isOpped() { return op; }
        bool isModified() { return mod_tool; }
        void setReady(bool flag) { ready = flag; }
        bool isReady() { return ready; }
        void setVoted(bool flag) { voted = flag; }
        bool isVoted() { return voted; }



        bool identity(Packet &packet);
        bool identity_process(const std::string & addr, bool is_banned, uint64_t timeout, bool do_timeout);
        bool message_received(Packet &packet);
        void disconnect(DisconnectReason reason, const std::string& message = "");
    };
}

#endif //DISASTERSERVER_PEER_HPP
