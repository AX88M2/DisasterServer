#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Io/Threads.hpp>
#include <cstdint>

using FilePtr = FILE*;

#define CONFIG_FILE "Config.json"

class ConfigManager {
    struct NetworkObject {
        int32_t port = 8606;
        int32_t server_count = 1;
    };

    struct PairingObject {
        int32_t ping_limit = 250;
    };

    struct LoggerObject {
        bool log_file = false;
    };

    struct GameplayObject {
        bool anticheat = true;
        bool map_list[20] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };
        Mutex map_list_lock;
    };

    struct MainObject {
        NetworkObject networking = {};
        PairingObject pairing = {};
        LoggerObject logger = {};
        GameplayObject gameplay = {};
        bool charFix = true;
        char motd[0x100] = "";
    };

    MainObject config = {};
public:
    ConfigManager();
    ~ConfigManager();

    bool loadConfig();
    bool saveConfig();

    MainObject getConfig();
};


#endif //CONFIG_HPP
