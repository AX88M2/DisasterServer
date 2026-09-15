#pragma once

#include "Util/Packet.hpp"
#include "States/State.hpp"
#include "Core/Constansts.hpp"
#include "Core/Assert.hpp"
#include "Core/Types.hpp"

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
        Server &server;

        std::unique_ptr<State> current;
    public:
        explicit StateController(Server &server);
        ~StateController();

        /**
         * \warning Данный метод не стоит сразу вызывать в @memberof State::enter,
         * и за этого может быть странное по видения.
         * @tparam T Класс наследований от @interface State <States\State.hpp>
         * @param args Дополнительные параметры для стадии
         */
        template <std::derived_from<State> T, typename... Args>
        void changeTo(Args&&... args) {
            auto next = std::make_unique<T>(server, *this, std::forward<Args>(args)...);

            next->enter();

            if (current) {
                current->exit();
            }

            current = std::move(next);
        }

        template <std::derived_from<State> T>
        bool isState() const {
            return current && dynamic_cast<T*>(current.get()) != nullptr;
        }

        bool playerJoined(Client& client);
        void playerLeft(Client& client);
        void tick();
        bool handle(Client& client, Packet& packet);

        commandHash cmdParse(std::string string);
        bool cmdHandle(Client& client, commandHash hash, const std::string& message);
    };
}

