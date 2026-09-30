#include "Configuration.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "Core/Log.hpp"

using namespace DisasterServer;

std::string Configuration::defaultConfig =R"([server]
port = 8606
lobby-count = 1
motd = "Hello from DisasterServerCXX"
)";

Configuration::Configuration() {
    Info("ConfigManager initialized...");
}

Configuration::~Configuration() = default;

void Configuration::load() {
    if (!std::filesystem::exists(filename)) {
        std::ofstream file {filename};
        file << defaultConfig;
        file.close();
    }

    try {
        Info("Loading configuration file...");
        toml = toml::parse(filename, toml::spec::v(1,1,0));
        config_ = Config(toml);
    } catch (const toml::syntax_error& err) {
        Error("Failed parse config file: {}", err.what());
        throw;
    }
}

void Configuration::save() {
    std::ofstream file {filename};
    file << toml::format(toml);
    file.close();
}