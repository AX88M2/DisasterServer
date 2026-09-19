#pragma once
#include <cstdint>
#include <string>

namespace DisasterServer {
    class Server;

    class Entity {
    public:
        Entity(std::string t) : tag(std::move(t)) {}
        virtual ~Entity() = default;
        virtual bool init(Server&){ return true; }
        virtual bool tick(Server&){ return true; }
        virtual bool uninit(Server&){ return true; }
        uint16_t id = 0;
        std::string tag;
    };
}