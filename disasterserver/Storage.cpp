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
    std::lock_guard lock(mutex);

    try {
        impl->storage.begin_transaction();
        impl->storage.insert(ClientBan(client.getIp(), client.getUdid(), client.getNickname(), reason));
        impl->storage.commit();
    } catch (std::exception &ex) {
        try { impl->storage.rollback(); } catch (...) {}
        Error("Failed to save for some reason: {}", ex.what());
    }
}

bool Storage::isBanned(Client &client) {
    std::lock_guard lock(mutex);

    try {
        const auto rows = impl->storage.select(
            columns(&ClientBan::ip, &ClientBan::uid),
            where(is_equal(&ClientBan::ip, client.getIp()) and is_equal(&ClientBan::uid, client.getUdid()))
        );

        return !rows.empty();
    } catch (std::exception &ex) {
        Error("Failed to read for some reason: {}", ex.what());
        return true;
    }
}
