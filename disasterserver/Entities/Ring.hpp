#pragma once
#include <cstdint>
#include "Entity.hpp"

namespace DisasterServer {

class Server;

class Ring : public Entity {
public:
    Ring() : Entity("ring") {}

    bool init(Server& server) override;
    bool uninit(Server& server) override;

    uint8_t rid = 0;
    uint8_t red = 0;
};

}