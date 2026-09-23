#pragma once

#include "Client.hpp"
#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Maps/Map.hpp"
#include "Util/Countdown.hpp"
#include "Packet.hpp"

#include "Core/Types.hpp"

namespace DisasterServer
{
    class StateController;

    class CharSelectState : public State {
        Countdown countdown { TICKS_PER_SEC };

        Map *map = nullptr;
        mapId mapid = 0;
        clientId exe = 0;
        std::unordered_map<SurvCharacters, bool> avail;
    public:
        CharSelectState(Server &server, StateController &stateController, Map* map, mapId id);
        ~CharSelectState() override;

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