#ifndef DISASTERSERVER_CHARSELECTSTATE_HPP
#define DISASTERSERVER_CHARSELECTSTATE_HPP

#include "Client.hpp"
#include "Core/State.hpp"
#include "Core/Packet.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{

class StateController;

class CharSelectState : public State {

    double countdown = 0;
    uint8_t countdownSec = 30;

    int8_t map = 0;
    clientId exe = 0;
    std::unordered_map<SurvCharacters, bool> avail;

public:
    CharSelectState(Server* server, StateController* controller);
    ~CharSelectState() = default;

    bool init(int8_t map);

    bool joined(Client& client) override;
    bool leaved(Client& client) override;
    void tick() override;
    bool handle(Client& client, Packet& packet) override;

    clientId getExe() const { return exe; }
    int8_t getMap() const { return map; }

    CharSelectState &get() { return *this; }
private:
    bool checkState();
    bool chooseExe();
};

}

#endif // DISASTERSERVER_CHARSELECTSTATE_HPP