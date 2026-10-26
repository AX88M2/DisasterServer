#pragma once

#include <string>

namespace DisasterServer
{
    class GameState;
    class Server;
    class Client;
    class Player;
    class Packet;

    class Character {
    protected:
        Server &server;
        Client &client;
    private:
        std::string name;
        bool exe;
    public:

        /**
         * @param server Ссылка на контекст сервера
         * @param client Ссылка на контекст клиента
         * @param name Имя персонажа
         * @param isExe Является ли персонаж киллером
         */
        Character(Server &server, Client &client, const std::string& name, const bool isExe = false) :
            server(server), client(client), name(name), exe(isExe)
        {}

        virtual ~Character() = default;

        virtual void tick() {}

        virtual void demonize() {}

        virtual bool handle(GameState&, Packet&) { return true; }

        std::string getName() const noexcept { return name; }
        bool isExe() const noexcept { return exe; }
    };
}
