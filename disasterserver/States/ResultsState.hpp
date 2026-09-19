#ifndef DISASTERSERVER_RESULTSSTATE_HPP
#define DISASTERSERVER_RESULTSSTATE_HPP

#include <memory>
#include <vector>

#include "State.hpp"
#include "Core/Constansts.hpp"
#include "Core/Types.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer
{
    enum class Ending : uint8_t;

    class ResultsState : public State {
        Countdown countdown { TICKS_PER_SEC };

        mapId id;
        uint16_t mapTimeSec;
        clientId exe;
        Ending ending;
        std::vector<std::unique_ptr<Client>> leftClients;
    public:
        ResultsState(Server &server, StateController &stateController, clientId exe, Ending ending, mapId id,
            uint16_t mapTimeSec,std::vector<std::unique_ptr<Client>> leftClients
        );
        ~ResultsState() override;

        void enter() override;
        void exit() override;
        bool playerJoined(Client& client) override;
        bool playerLeaved(Client& client) override;
        void tick() override;
        bool handle(Client& client, Packet& packet) override;
    private:

        bool sendResult(Client& client, Client& data, bool hasQuit);
    };
}

#endif //DISASTERSERVER_RESULTSSTATE_HPP
