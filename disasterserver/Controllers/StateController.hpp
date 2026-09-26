#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <ranges>

#include "Packet.hpp"
#include "States/State.hpp"
#include "Core/Constansts.hpp"
#include "Core/Assert.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{
    class Client;

    enum class Commands : commandHash {
        HELP = 45680751,
        MAP = 1478254,
        STINK = 1426706039,
        VK = 47971,
        VP = 47976,
        BAN = 1467681,
        KICK = 45773684,
        OP = 47759,
        YES = 1489913,
        Y = 1547,
        NO = 47727,
        N = 1536,
        INFO = 45719004,
        LOBBY = 1420085352,
        SELFOP = 1264443355,
#if defined(SERVER_DEBUG)
        DEBUG = 1412399845,
#endif
    };

    class StateController {
        friend class Client;
        friend class Server;

        Server &server;

        std::vector<std::unique_ptr<State>> pendingState = {};
        std::unique_ptr<State> current;
    public:
        explicit StateController(Server &server);
        ~StateController();

        /**
         * @tparam T Класс наследований от @interface State <States\State.hpp>
         * @param args Дополнительные параметры для стадии
         */
        template <std::derived_from<State> T, typename... Args>
        void changeTo(Args&&... args) {
            auto next = std::make_unique<T>(server, *this, std::forward<Args>(args)...);
            pendingState.emplace_back(std::move(next));
        }

        template <std::derived_from<State> T>
        bool isState() const {
            return current && dynamic_cast<T*>(current.get()) != nullptr;
        }

        template <std::derived_from<State> T>
        T* getState() const {
            return static_cast<T*>(current.get());
        }

        Commands cmdParse(std::string string);

        void handleChat(Client &client, std::string& message, std::function<bool(Commands, std::string &)> cmdProcessor);
        bool cmdHandle(Client& client, Commands hash, const std::string& message);
    private:
        /*** === Events === ***/
        bool playerJoined(Client& client);
        void playerLeft(Client& client);
        void tick();
        bool handle(Client& client, Packet& packet);
    };
}

