#ifndef DISASTERSERVER_CHARSELECTSTATE_HPP
#define DISASTERSERVER_CHARSELECTSTATE_HPP

#include "Client.hpp"
#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Core/Map.hpp"
#include "Util/Countdown.hpp"
#include "Util/Packet.hpp"
#include "Core/Types.hpp"

namespace DisasterServer
{

class StateController;

class CharSelectState : public State {
    Countdown countdown = Countdown(TICKSPERSEC);

    Map *map = nullptr;
    int8_t mapId = 0;
    clientId exe = 0;
    std::unordered_map<SurvCharacters, bool> avail;
public:
    CharSelectState(Server* server, StateController* controller);
    ~CharSelectState() override = default;

    void init(Map* map, uint8_t id);

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