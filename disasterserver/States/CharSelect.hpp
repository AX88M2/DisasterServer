#ifndef DISASTERSERVER_CHARSELECTSTATE_HPP
#define DISASTERSERVER_CHARSELECTSTATE_HPP

#include "Client.hpp"
#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Maps/Map.hpp"
#include "Util/Countdown.hpp"
#include "Util/Packet.hpp"

#include "Core/Types.hpp"

namespace DisasterServer
{

class StateController;

class CharSelectState : public State {
    Countdown countdown = Countdown(TICKSPERSEC);

    Map *map = nullptr;
    mapId mapid = 0;
    clientId exe = 0;
    std::unordered_map<SurvCharacters, bool> avail;
public:
    CharSelectState(Server &server, StateController &stateController, Map* map, mapId id);
    ~CharSelectState() override = default;

    void enter() override;
    void exit() override;
    bool playerJoined(Client& client) override;
    bool playerLeaved(Client& client) override;
    void tick() override;
    bool handle(Client& client, Packet& packet) override;
private:
    bool checkState();
    bool chooseExe();
};
}
#endif