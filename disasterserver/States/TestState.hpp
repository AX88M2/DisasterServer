#ifndef DISASTERSERVER_TESTSTATE_HPP
#define DISASTERSERVER_TESTSTATE_HPP

#include "Core/State.hpp"

namespace DisasterServer
{
    class TestState : public State {
        Server *server = nullptr;
        StateController *controller = nullptr;
    public:
        TestState(Server *server, StateController *controller, int test = 0) : State(server, controller) {}

        ~TestState() override {

        }

        bool joined(Client &client) override {
            return true;
        }

        bool leaved(Client &client) override {
            return true;
        };

        void tick() override {
        };

        void handle(Client &client, Packet &packet) override {
        };
    };
}



#endif //DISASTERSERVER_TESTSTATE_HPP
