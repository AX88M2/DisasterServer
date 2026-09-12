#ifndef DISASTERSERVER_STATEMACHINE_HPP
#define DISASTERSERVER_STATEMACHINE_HPP

#include "Util/Packet.hpp"
#include "States/State.hpp"
#include "Core/Constansts.hpp"

namespace DisasterServer
{
    class Client;

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

    class StateController {
        Server *server = nullptr;

        std::unique_ptr<State> current;
    public:
        explicit StateController(Server* server);
        ~StateController();

        template <std::derived_from<State> T, typename... Args>
        requires requires (T& state, Args&&... args) { state.init(std::forward<Args>(args)...); }
        void changeTo(Args&&... args) {
            auto next = std::make_unique<T>(server, this);
            next->init(std::forward<Args>(args)...);
            current = std::move(next);
        }

        template <std::derived_from<State> T>
        bool isState() const {
            return dynamic_cast<T*>(current.get()) != nullptr;
        }

        bool playerJoined(Client& peer);
        void playerLeft(Client& peer);
        void tick();
        bool handle(Client& peer, Packet& packet);

        commandHash cmdParse(std::string string);
        bool cmdHandle(Client& client, commandHash hash, const std::string& message);
    };
}

#define AssertOrDisconnect(client, x) \
if(!(x)) { \
    client.disconnect(DisconnectReason::OTHER, "AssertOrDisconnect({}) failed!", #x); return false; \
}

#endif //DISASTERSERVER_STATEMACHINE_HPP

