#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <vector>

namespace DisasterServer
{
    class Client;

    class ClientBan {
    public:
        std::string ip;
        std::string uid;
        std::string username;

        std::string reason;

        explicit ClientBan() : ip(""), uid(""), username(""), reason("") {}
        explicit ClientBan(const std::string &ip, const std::string &uid, const std::string &username, const std::string &reason) :
            ip(ip), uid(uid), username(username), reason(reason) {}
        ~ClientBan() = default;
    };

    class ClientOperator {
    public:
        std::string ip;
        std::string uid;

        explicit ClientOperator() : ip(""), uid("") {}
        explicit ClientOperator(const std::string &ip, const std::string &uid) : ip(ip), uid(uid) {}
        ~ClientOperator() = default;
    };

    class Storage {
        struct Impl;
        std::unique_ptr<Impl> impl;
        std::mutex mutex;
    public:
        explicit Storage();
        ~Storage();

        void addBan(Client &client, const std::string &reason = "");
        void removeBan(Client &client);
        bool isBanned(Client &client);
        std::vector<ClientBan> getBans();

        void addOperator(Client &client);
        void removeOperator(Client &client);
        bool isOperator(Client &client);
        std::vector<ClientOperator> getOperators();
    private:
        template <typename... Args> void write(Args&&... args);
        template<typename T, typename... Args> void remove(Args&&... args);
    };
}
