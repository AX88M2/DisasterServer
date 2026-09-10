#ifndef DISASTERSERVER_CHARSELECTSTATE_HPP
#define DISASTERSERVER_CHARSELECTSTATE_HPP

#include "Client.hpp"
#include "State.hpp"
#include "Util/Countdown.hpp"
#include "Core/Packet.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{
class StateController;

class CharSelectState : public State {
    Countdown countdown;

    int map = 0;
    clientId exe = 0;
    std::unordered_map<SurvCharacters, bool> avail;
public:
    static constexpr States STATE_ID = States::CHARSELECT;

    CharSelectState(Server* server, StateController* controller);
    ~CharSelectState() = default;

    void init(int selectedMap);

    bool joined(Client& client) override;
    bool leaved(Client& client) override;
    void tick() override;
    bool handle(Client& client, Packet& packet) override;
private:
    bool checkState();
    bool chooseExe();
};
}
#endif