#ifndef DISASTERSERVER_CHARSELECTSTATE_HPP
#define DISASTERSERVER_CHARSELECTSTATE_HPP

#include <array>
#include <cstdint>

#include "Client.hpp"
#include "Core/Packet.hpp"

namespace DisasterServer {

class GameStateController;

class CharSelectState {
    Server *server = nullptr;
    GameStateController* controller = nullptr;

    double countdown = 0;
    uint8_t countdownSec = 30;

    int8_t map = 0;
    clientId exe = 0;

    std::array<bool, 6> avail{};

    bool checkState();
    bool chooseExe();

public:
    CharSelectState(Server* server, GameStateController* controller);
    ~CharSelectState() = default;

    bool init(int8_t map);

    bool joined(Client& client);
    bool leaved(Client& client);
    void tick();
    bool handle(Client& client, Packet& packet);

    clientId getExe() const { return exe; }
    int8_t getMap() const { return map; }
};

}

#endif // DISASTERSERVER_CHARSELECTSTATE_HPP