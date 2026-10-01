#pragma once

#include <optional>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

#include "Core/Exceptions.hpp"
#include "Core/Log.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{
    class CommandController;
    class StateController;
    class Server;
    class Client;

    class CommandArguments {
        std::vector<std::string> arguments;
    public:
        explicit CommandArguments(std::vector<std::string> arguments) : arguments(arguments) {}

        /**
         * @tparam T Тип данных
         * @param id Порядковый id аргумента (Начитается с 0)
         * @param defaultValue Значения по умолчанию
         * @return Возвращает значения аргумента
         */
        template <typename T>
        std::optional<T> get(size_t id, std::optional<T> defaultValue = std::nullopt) {

            if (id < arguments.size()) {

                try {
                   return boost::lexical_cast<T>(arguments[id]);
                } catch (boost::bad_lexical_cast &e) {
                    Error("Failed get argument: {}", e.what());
                    return defaultValue;
                } catch (std::exception &e) {
                    Error("{}", e.what());
                    return defaultValue;
                }

            } else {
                return defaultValue;
            }
        }
    };

    class Command {
        friend class CommandController;
    protected:
        Server &server;
        StateController &stateController;
    private:
        std::string name;
    public:
        explicit Command(Server &server, StateController &stateController, std::string command) :
            server(server), stateController(stateController), name(std::move(command)) {}

        virtual ~Command() = default;

        virtual void execute(Client &client, CommandArguments &args) = 0;
    };
}
