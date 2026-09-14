#include "ResultsState.hpp"

using namespace DisasterServer;

ResultsState::ResultsState(Server *server, StateController *controller) : State(server, controller) {
}

void ResultsState::init() {

}

bool ResultsState::joined(Client &client) {
    return State::joined(client);
}

bool ResultsState::leaved(Client &client) {
    return State::leaved(client);
}

void ResultsState::tick() {
    State::tick();
}

bool ResultsState::handle(Client &client, Packet &packet) {
    return State::handle(client, packet);
}
