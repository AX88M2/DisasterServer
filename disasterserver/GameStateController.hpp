#ifndef DISASTERSERVER_STATEMACHINE_HPP
#define DISASTERSERVER_STATEMACHINE_HPP

#include "Core/Packet.hpp"
#include "States/LobbyState.hpp"
#include "States/CharSelect.hpp"

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

    constexpr commandHash CMD_HELP = 45680751;
    constexpr commandHash CMD_MAP = 1478254;
    constexpr commandHash CMD_STINK = 1426706039;
    constexpr commandHash CMD_VK = 47971;
    constexpr commandHash CMD_VP = 47976;
    constexpr commandHash CMD_BAN = 1467681;
    constexpr commandHash CMD_KICK = 45773684;
    constexpr commandHash CMD_OP = 47759;
    constexpr commandHash CMD_YES = 1489913;
    constexpr commandHash CMD_Y = 1547;
    constexpr commandHash CMD_NO = 47727;
    constexpr commandHash CMD_N = 1536;
    constexpr commandHash CMD_INFO = 45719004;
    constexpr commandHash CMD_LOBBY = 1420085352;

    constexpr commandHash CMD_SELFOP = 1264443355; //Only debugging
    constexpr commandHash CMD_DEBUG = 1412399845; //Only debugging

    class GameStateController {
        Server *server = nullptr;
        States state = States::LOBBY;

        LobbyState lobby;
        CharSelectState charSelect;
    public:
        explicit GameStateController(Server* server);
        ~GameStateController();

        bool playerJoined(Client& peer);
        void playerLeft(Client& peer);
        void tick();
        bool handle(Client& peer, Packet& packet);

        commandHash cmd_parse(std::string& string);
        bool cmd_handle(Client& client, commandHash hash, const std::string& message);

        States getCurrentState() const { return state; }
        void setState(States state) { this->state = state; }


        LobbyState& getLobbyState() { return lobby; }
        CharSelectState& getCharSelect() { return charSelect; }
    };
}

#endif //DISASTERSERVER_STATEMACHINE_HPP
