#include "Storage.hpp"

#include "Client.hpp"
#include "Core/Log.hpp"
#include <sqlite_orm/sqlite_orm.h>

using namespace DisasterServer;
using namespace sqlite_orm;

namespace
{
    constexpr auto filename = "database.db";

    auto createStorage() {
        return make_storage(
            filename,
            make_table<ClientBan>("bans",
                make_column("ip", &ClientBan::ip),
                make_column("uid", &ClientBan::uid),
                make_column("username", &ClientBan::username),
                make_column("reason", &ClientBan::reason)
            ),
            make_table<ClientOperator>("operators",
                make_column("ip", &ClientOperator::ip),
                make_column("uid", &ClientOperator::uid)
            )
        );
    }

    using StorageType = decltype(createStorage());
}

struct Storage::Impl {
    StorageType storage;
    Impl() : storage(createStorage()) { storage.sync_schema(); }
};

Storage::Storage() : impl(std::make_unique<Impl>()) {
    Info("Storage initialized...");
}

Storage::~Storage() = default;

void Storage::addBan(Client &client, const std::string &reason) {
    write(ClientBan(client.getIp(), client.getUdid(), client.getNickname(), reason));
}

void Storage::removeBan(Client &client) {
    remove<ClientBan>(where(is_equal(&ClientBan::ip, client.getIp()) and is_equal(&ClientBan::uid, client.getUdid())));
}

bool Storage::isBanned(Client &client) {
    std::lock_guard lock(mutex);

    try {
        const auto rows = impl->storage.select(columns(&ClientBan::ip, &ClientBan::uid),
            where(is_equal(&ClientBan::ip, client.getIp()) and is_equal(&ClientBan::uid, client.getUdid()))
        );

        return !rows.empty();
    } catch (std::exception &ex) {
        Error("Failed to read for some reason: {}", ex.what());
        return true;
    }
}

std::vector<ClientBan> Storage::getBans() {
    std::lock_guard lock(mutex);

    try {
        auto objects = impl->storage.get_all<ClientBan>();
        return objects;
    } catch (std::exception &ex) {
        Error("Failed to read for some reason: {}", ex.what());
        return {};
    }
}

void Storage::addOperator(Client &client) {
    write(ClientOperator(client.getIp(), client.getUdid()));
}

void Storage::removeOperator(Client &client) {
    remove<ClientOperator>(where(is_equal(&ClientOperator::ip, client.getIp()) and is_equal(&ClientOperator::uid, client.getUdid())));
}

bool Storage::isOperator(Client &client) {
    std::lock_guard lock(mutex);

    try {
        const auto rows = impl->storage.select(columns(&ClientOperator::ip, &ClientOperator::uid),
            where(is_equal(&ClientOperator::ip, client.getIp()) and is_equal(&ClientOperator::uid, client.getUdid()))
        );

        return !rows.empty();
    } catch (std::exception &ex) {
        Error("Failed to read for some reason: {}", ex.what());
        return true;
    }
}

std::vector<ClientOperator> Storage::getOperators() {
    std::lock_guard lock(mutex);

    try {
        auto objects = impl->storage.get_all<ClientOperator>();
        return objects;
    } catch (std::exception &ex) {
        Error("Failed to read for some reason: {}", ex.what());
        return {};
    }
}

template<typename... Args>
void Storage::write(Args &&... args) {
    std::lock_guard lock(mutex);

    try {
        impl->storage.begin_transaction();
        impl->storage.insert(std::forward<Args>(args)...);
        impl->storage.commit();
    } catch (std::exception &ex) {
        try { impl->storage.rollback(); } catch (...) {}
        Error("Failed to save for some reason: {}", ex.what());
    }
}

template<typename T, typename... Args>
void Storage::remove(Args &&... args) {
    std::lock_guard lock(mutex);

    try {
        impl->storage.begin_transaction();
        impl->storage.remove_all<T>(std::forward<Args>(args)...);
        impl->storage.commit();
    } catch (std::exception &ex) {
        try { impl->storage.rollback(); } catch (...) {}
        Error("Failed to save for some reason: {}", ex.what());
    }
}

