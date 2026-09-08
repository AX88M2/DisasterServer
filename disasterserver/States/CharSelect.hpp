#ifndef DISASTERSERVER_CHARSELECTSTATE_HPP
#define DISASTERSERVER_CHARSELECTSTATE_HPP

#include "Client.hpp"
#include "State.hpp"
#include "Core/Packet.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{

class GameStateController;

class CharSelectState : public State<CharSelectState> {

    double countdown = 0;
    uint8_t countdownSec = 30;

    int8_t map = 0;
    clientId exe = 0;
    std::unordered_map<SurvCharacters, bool> avail;

public:
    CharSelectState(Server* server, GameStateController* controller);
    ~CharSelectState() = default;

    bool init(int8_t map);

    bool joined(Client& client) override;
    bool leaved(Client& client) override;
    bool tick() override;
    bool handle(Client& client, Packet& packet) override;

    clientId getExe() const { return exe; }
    int8_t getMap() const { return map; }

    CharSelectState &get() override { return *this; }
private:
    bool checkState();
    bool chooseExe();
};

}

#endif // DISASTERSERVER_CHARSELECTSTATE_HPP