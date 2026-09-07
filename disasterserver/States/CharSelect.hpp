#ifndef DISASTERSERVER_CHARSELECTSTATE_HPP
#define DISASTERSERVER_CHARSELECTSTATE_HPP

#include <array>
#include <cstdint>

#include "State.hpp"
#include "Core/Packet.hpp"

namespace DisasterServer {

class GameStateController;

class CharSelectState : public State<CharSelectState> {
private:
    GameStateController* controller = nullptr;

    double countdown = 0;
    uint8_t countdownSec = 30;

    int8_t map = 0;
    clientId exe = 0;

    std::array<bool, 6> avail{};

    bool checkState();
    bool chooseExe();

    bool isValidSurvivorCharacter(uint8_t id) const;
    bool isValidExeCharacter(uint8_t id) const;

public:
    CharSelectState(Server* server, GameStateController* controller);

    ~CharSelectState() override = default;

    bool init(int8_t map);

    void tick() override;

    bool handle(Client& client, Packet& packet) override;

    bool joined(Client& client) override;
    bool leaved(Client& client);

    clientId getExe() const { return exe; }
    int8_t getMap() const { return map; }
};

}

#endif // DISASTERSERVER_CHARSELECTSTATE_HPP