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

    enum class CommandsHash : commandHash {
        VK = 47971,
        VP = 47976,
        YES = 1489913,
        Y = 1547,
        NO = 47727,
        N = 1536,
        EXE = 1471268,
    };

    class StateController {
        friend class Client;
        friend class Server;

        Server &server;
        MapController &mapController;

        std::vector<std::unique_ptr<State>> pendingState = {};
        std::unique_ptr<State> current;
    public:
        explicit StateController(Server &server, MapController &mapController);
        ~StateController();

        /**
         * @tparam T Класс наследований от @interface State <States\State.hpp>
         * @param args Дополнительные параметры для стадии
         */
        template <std::derived_from<State> T, typename... Args>
        void changeTo(Args&&... args) {
            auto next = std::make_unique<T>(server, ContextControllers {*this, mapController}, std::forward<Args>(args)...);
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

        CommandsHash cmdParse(std::string string);

        void handleChat(Client &client, std::string& message, std::function<bool(CommandsHash, std::string &)> cmdProcessor);
        bool cmdHandle(Client& client, CommandsHash hash, const std::string& msg);
    private:
        /*** === Events === ***/
        bool playerJoined(Client& client);
        void playerLeft(Client& client);
        void tick();
        bool handle(Client& client, Packet& packet);
    };
}

