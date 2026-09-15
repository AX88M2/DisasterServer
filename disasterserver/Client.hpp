#ifndef DISASTERSERVER_PEER_HPP
#define DISASTERSERVER_PEER_HPP

#include "Player.hpp"
#include "Controllers/StateController.hpp"
#include "Util/Packet.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{
    class Server;

    enum class SurvCharacters {
        NONE = -1,

        TAILS,
        KNUX,
        EGGMAN,
        AMY,
        CREAM,
        SALLY,
        SONIC,
        COUNT
    };

    enum class ExesCharacters {
        NONE = -1,

        ORIGINAL,
        CHAOS,
        EXETIOR,
        EXELLER,
        COUNT
    };

    static std::vector<std::string_view> EXE_NAMES = {
        "Classic Exe",
        "Chaos",
        "Exetior",
        "Exeller"
    };

    static std::vector<std::string_view> SURV_NAMES = {
        "Tails",
        "Knuckles",
        "Eggman",
        "Amy",
        "Cream",
        "Sally",
        "Sonic"
    };

    enum class DisconnectReason : uint8_t
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
            case DisconnectReason::IPINUSE: return "IP IN USE";
            case DisconnectReason::DONTREPORT: return "DONT REPORT";
            case DisconnectReason::OTHER: return "OTHER";
            default: return "<unknown>";
        }
    }


    class Client {
        clientId id;
        std::string ip;
        ENetPeer *peer;

        /* General info */
        Player player = {};
        std::string nickname = "<unknown>";
        std::string udid;
        uint8_t lobbyIcon = 0;
        int8_t pet = -1;

        bool verified = false;
        bool in_game = false;
        bool op = false;
        bool ready = false;
        bool isModifiedClient = false;
        bool can_vote = false;
        bool voted = false;
        bool disconnecting = false;

        struct AuthPeer {
            uint32_t type = 0;
            uint8_t	 one = 0;
            uint8_t	 two = 0;
        } auth = {};

        /* Character */
        SurvCharacters survChar = SurvCharacters::NONE;
        ExesCharacters exeChar = ExesCharacters::NONE;

        bool shouldTimeout = false;

        /* State info */
        uint8_t exeChance = 0;
        double timeout = 0;
        double voteCooldown = 0;

        Server *server = nullptr;
        StateController &stateController;
    public:
        Client(Server *server, StateController &stateController, ENetPeer *peer, clientId incomingPeerID, std::string ip);
        ~Client();

        clientId getId() const { return id; }
        std::string getIp() const { return ip; }
        ENetPeer *getPeer() const { return peer; }
        Player &getPlayer() { return player; }
        std::string getNickname() { return nickname; }
        std::string getUdid() { return udid; }
        uint8_t getLobbyIcon() const { return lobbyIcon; }
        int8_t getPet() const { return pet; }
        AuthPeer &getAuthPeer() { return auth; }
        void setExeChance(const uint8_t chance) { exeChance = chance; }
        uint8_t getExeChance() const { return exeChance; }

        SurvCharacters getSurvCharacter() const { return survChar; }
        void setSurvCharacter(SurvCharacters character) { survChar = character; }
        ExesCharacters getExeCharacter() const { return exeChar; }
        void setExeCharacter(ExesCharacters character) { exeChar = character; }

        void setTimeout(const double value) { timeout = value; }
        double getTimeout() const { return timeout; }
        bool isShouldTimeout() const { return shouldTimeout; }
        bool isDisconnecting() const { return disconnecting; }
        void setVoteCooldown(const double value) { voteCooldown = value; }
        double getVoteCooldown() const { return voteCooldown; }
        void setInGame(const bool flag) { in_game = flag; }
        bool isInGame() const { return in_game; }
        void setCanVote(const bool flag) { can_vote = flag; }
        bool isCanVote() const { return can_vote; }
        void setOperator(const bool flag) { op = flag; }
        bool isOperator() const { return op; }

        bool isVerified() const { return verified; }
        bool isModified() const { return isModifiedClient; }
        void setReady(const bool flag) { ready = flag; }
        bool isReady() const { return ready; }
        void setVoted(const bool flag) { voted = flag; }
        bool isVoted() const { return voted; }

        bool identity(Packet &packet);
        bool identityProcess(const std::string &addr, bool is_banned, uint64_t timeout, bool do_timeout);
        bool messageReceived(Packet &packet);
        void disconnect(DisconnectReason reason, const std::string& message = "");

        template <typename... Args>
        void disconnect(const DisconnectReason reason, std::format_string<Args...> fmt, Args&&... args) {
            disconnect(reason, std::format(fmt, std::forward<Args>(args)...));
        }
    };
}

#endif //DISASTERSERVER_PEER_HPP
