#pragma once
#include <cstdint>
#include "Entity.hpp"

namespace DisasterServer {

class Server;

class Ring : public Entity {
public:
    Ring(uint16_t id, Server &server, StateController &stateController);

    bool init() override;
    bool uninit() override;

    uint8_t rid = 0;
    uint8_t red = 0;
};

}