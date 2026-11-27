#pragma once

#include "Player.hpp"
#include "Controllers/StateController.hpp"
#include "Packet.hpp"
#include "Characters/Character.hpp"
#include "Characters/None.hpp"

namespace DisasterServer
{
    class PacketHandler;

    enum class SurvivalCharacters {
        NONE = -1,

        TAILS,
        KNUCKLES,
        EGGMAN,
        AMY,
        CREAM,
        SALLY,

        COUNT
    };

    enum class ExeCharacters {
        NONE = -1,

        ORIGINAL,
        CHAOS,
        EXETIOR,
        EXELLER,

        COUNT
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

    inline std::string toStringDisconnectReason(DisconnectReason reason) {
        switch (reason) {
            case DisconnectReason::FAILEDTOCONNECT: return "FAILED TO CONNECT";
            case DisconnectReason::KICKEDBYHOST: return "KICKED BY HOST";
            case DisconnectReason::BANNEDBYHOST: return "BANNED BY HOST";
            case DisconnectReason::VERMISMATCH: return "VER MISMATCH";
            case DisconnectReason::SERVERTIMEOUT: return "SERVER TIMEOUT";
            case DisconnectReason::PACKETSNOTRECV: return "PACKETS NOT RECV";
            case DisconnectReason::GAMESTARTED: return "GAME STARTED";
            case DisconnectReason::AFKTIMEOUT: return "AFK TIMEOUT";
            case DisconnectReason::LOBBYFULL: return "LOBBY FULL";
            case DisconnectReason::RATELIMITED: return "RATE LIMITED";
            case DisconnectReason::SHUTDOWN: return "SHUTDOWN";
            case DisconnectReason::IPINUSE: return "IP IN USE";
            case DisconnectReason::DONTREPORT: return "DON'T REPORT";
            case DisconnectReason::OTHER: return "OTHER";
            default: return "<unknown>";
        }
    }

    struct TicketAuth {
        uint32_t type = 0;
        uint8_t	 one = 0;
        uint8_t	 two = 0;
    };

    class Client {
        friend class Server;

        clientId id;
        std::string ip;
        ENetPeer *peer;

        /* General info */
        std::shared_ptr<Player> player = std::make_shared<Player>();
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

        TicketAuth ticketAuth = {};

        /* Character */
        SurvivalCharacters survChar = SurvivalCharacters::NONE;
        ExeCharacters exeChar = ExeCharacters::NONE;
        std::shared_ptr<Character> character;

        bool shouldTimeout = false;

        /* State info */
        uint8_t exeChance = 0;
        double afkTimeout = 0;
        double voteCooldown = 0;

        Server *server = nullptr;
        StateController &stateController;
    public:
        Client(Server *server, StateController &stateController, ENetPeer *peer, clientId incomingPeerID, std::string ip);
        Client(Client const&) = default;
        ~Client() = default;

        clientId getId() const { return id; }
        std::string getIp() const { return ip; }
        ENetPeer *getPeer() const { return peer; }

        Player &getPlayer() const { return *player; }

        void resetPlayer();

        std::string getNickname() { return nickname; }
        std::string getUdid() { return udid; }
        uint8_t getLobbyIcon() const { return lobbyIcon; }
        int8_t getPet() const { return pet; }
        TicketAuth &getTicketAuth() { return ticketAuth; }
        void setExeChance(const uint8_t chance) { exeChance = chance; }
        uint8_t getExeChance() const { return exeChance; }

        SurvivalCharacters getSurvCharacter() const { return survChar; }
        void setSurvCharacter(const SurvivalCharacters ch) { survChar = ch; }
        ExeCharacters getExeCharacter() const { return exeChar; }
        void setExeCharacter(const ExeCharacters ch) { exeChar = ch; }

        std::shared_ptr<Character> getCharacter() const { return character; }
        template <std::derived_from<Character> T>
        void setCharacter() {
            character = std::move(std::make_shared<T>(*server, *this));
        }
        template <std::derived_from<Character> T>
        bool isCharacter() const {
            return dynamic_cast<T *>(character.get()) != nullptr;
        }

        void setAfkTimeout(const double value) { afkTimeout = value; }
        double getAfkTimeout() const { return afkTimeout; }
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

        void disconnect(DisconnectReason reason, const std::string& message = "");
        template <typename... Args>
        void disconnect(const DisconnectReason reason, std::format_string<Args...> fmt, Args&&... args) {
            disconnect(reason, std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        bool identity(Packet &packet);
        bool identityProcess(const std::string &addr, bool isBanned, uint64_t timeout, bool doTimeout);
        bool received(Packet &packet);
    };
}