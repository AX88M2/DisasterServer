#pragma once

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
         * @return Возвращает значения аргумента
         */
        template <typename T>
        T get(size_t id) {
            T result;

            try {
                std::string temp = arguments[id];
                result = boost::lexical_cast<T>(temp);
            } catch (boost::bad_lexical_cast &e) {
                throw CommandException::format("Failed get argument: {}", e.what());
            } catch (std::exception &e) {
                throw CommandException(e.what());
            }

            return result;
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
            server(server), stateController(stateController), name(command) {}

        virtual ~Command() = default;

        virtual void execute(Client &client, CommandArguments &args) = 0;
    };
}
