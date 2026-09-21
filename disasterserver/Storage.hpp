#pragma once

#include <memory>
#include <string>
#include <mutex>

namespace DisasterServer
{
    class Client;

    class ClientBan {
    public:
        std::string ip;
        std::string uid;
        std::string username;

        std::string reason;

        explicit ClientBan(const std::string &ip, const std::string &uid, const std::string &username, const std::string &reason) : ip(ip), uid(uid), username(username), reason(reason) {}
    };

    class Storage {
        struct Impl;
        std::unique_ptr<Impl> impl;
        std::mutex mutex;
    public:
        explicit Storage();
        ~Storage();

        void addBan(Client &client, const std::string &reason = "");
        bool isBanned(Client &client);
    };
}
