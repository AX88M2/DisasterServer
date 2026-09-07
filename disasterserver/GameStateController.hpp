#ifndef DISASTERSERVER_STATEMACHINE_HPP
#define DISASTERSERVER_STATEMACHINE_HPP

#include "Core/Packet.hpp"
#include "States/LobbyState.hpp"

namespace DisasterServer
{
    class Client;

    enum class States {
        LOBBY,
        MAPVOTE,
        CHARSELECT,
        GAME,
        RESULTS
    };

#define CMD_HELP 45680751
#define CMD_MAP 1478254
#define CMD_STINK 1426706039
#define CMD_VK 47971
#define CMD_VP 47976
#define CMD_BAN 1467681
#define CMD_KICK 45773684
#define CMD_OP 47759
#define CMD_YES 1489913
#define CMD_Y 1547
#define CMD_NO 47727
#define CMD_N 1536
#define CMD_INFO 45719004
#define CMD_LOBBY 1420085352

    using commandHash = unsigned long;

    class GameStateController {
        Server *server = nullptr;
        States state = States::LOBBY;

        LobbyState lobby;
    public:
        explicit GameStateController(Server *server);
        ~GameStateController();

        bool playerJoined(Client &peer);
        void playerLeft(Client &peer);
        void tick();
        bool handle(Client &peer, Packet &packet);

        commandHash cmd_parse(std::string &string);
        bool cmd_handle(const Client & client, commandHash hash, const std::string & string);

        States getCurrentState() const { return this->state; }
        void setState(States state) { this->state = state; }
    };
}

#endif //DISASTERSERVER_STATEMACHINE_HPP
