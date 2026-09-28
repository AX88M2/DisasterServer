#pragma once

#include <string>

namespace DisasterServer
{
    class GameState;
    class Server;
    class Client;
    class Player;
    class Packet;

    enum class CharacterType {
        TAILS,
        KNUCKLES,
        EGGMAN,
        AMY,
        CREAM,
        SALLY,
        SONIC,
    };

    class Character {
    protected:
        Server &server;
        Client &client;
        Player &player;
    private:
        CharacterType type;

        std::string name;
        bool exe;
    public:

        /**
         * @param server Ссылка на контекст сервера
         * @param client Ссылка на контекст клиента
         * @param type Тип
         * @param name Имя персонажа
         * @param isExe Является ли персонаж киллером
         */
        Character(Server &server, Client &client, Player &player,
            const CharacterType type,
            const std::string& name,
            const bool isExe = false
        ) : server(server), client(client), player(player), type(type), name(name), exe(isExe) {}

        virtual ~Character() {}

        virtual void tick() = 0;

        virtual bool handle(GameState&, Packet&) { return true; }

        template <std::derived_from<Character> T>
        bool is() const noexcept {
            return type == T::Type;
        }

        std::string getName() const noexcept { return name; }
        bool isExe() const noexcept { return exe; }
    };

}
