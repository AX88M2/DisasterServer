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

        explicit ClientBan(const std::string &ip, const std::string &uid, const std::string &username) : ip(ip), uid(uid), username(username) {}
    };

    class Storage {
        struct Impl;
        std::unique_ptr<Impl> impl;
        std::mutex mutex;
    public:
        explicit Storage();
        ~Storage();

        void addBan(Client &client);
    };
}
