#include "ConfigManager.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Core/Log.hpp"

using namespace DisasterServer;

std::string ConfigManager::defaultConfig =R"([server]
port = 8606
lobby-count = 1
motd = "Hello from DisasterServerCXX"
)";

ConfigManager::ConfigManager() = default;

ConfigManager::~ConfigManager() = default;


void ConfigManager::load() {
    if (!std::filesystem::exists(filename)) {
        Info("{} not found, creating default config", filename);
        std::ofstream file {filename};
        file << defaultConfig;
        file.close();
    }

    try {
        toml = toml::parse(filename, toml::spec::v(1,1,0));
        config = Config(toml);
    } catch (const toml::syntax_error& err) {
        Error("Failed parse config file: {}", err.what());
        throw;
    }
}

void ConfigManager::save() {
    std::ofstream file {filename};
    file << toml::format(toml);
    file.close();
}